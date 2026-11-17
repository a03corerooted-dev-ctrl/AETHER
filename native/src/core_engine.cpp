#include "aether/core_engine.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace aether {
namespace {

struct Signals final {
    bool valid{false};
    std::optional<double> available_ratio;
    std::optional<double> psi_some;
    std::optional<double> psi_full;
};

int severity(PressureBand band) noexcept {
    switch (band) {
        case PressureBand::normal: return 0;
        case PressureBand::moderate: return 1;
        case PressureBand::high: return 2;
        case PressureBand::critical: return 3;
        case PressureBand::unknown: return -1;
    }
    return -1;
}

bool finite_between(double value, double minimum, double maximum) noexcept {
    return std::isfinite(value) && value >= minimum && value <= maximum;
}

Signals collect_signals(const ObservationBatch& observation) {
    Signals signals{};

    if (observation.memory_valid && observation.memory.total_kib.has_value() &&
        observation.memory.available_kib.has_value() &&
        *observation.memory.total_kib > 0U &&
        *observation.memory.available_kib <= *observation.memory.total_kib) {
        signals.available_ratio =
            static_cast<double>(*observation.memory.available_kib) /
            static_cast<double>(*observation.memory.total_kib);
        signals.valid = true;
    }

    if (observation.pressure.some.has_value()) {
        signals.psi_some = observation.pressure.some->avg10;
        signals.valid = true;
    }
    if (observation.pressure.full.has_value()) {
        signals.psi_full = observation.pressure.full->avg10;
        signals.valid = true;
    }

    return signals;
}

int classify(const Signals& signals, const CoreConfig& config, bool recovering) noexcept {
    if (!signals.valid) return -1;

    const double ratio_margin = recovering ? config.recovery_available_margin : 0.0;
    const double psi_margin = recovering ? config.recovery_psi_margin : 0.0;

    const double moderate_ratio = std::min(1.0, config.moderate_available_ratio + ratio_margin);
    const double high_ratio = std::min(1.0, config.high_available_ratio + ratio_margin);
    const double critical_ratio = std::min(1.0, config.critical_available_ratio + ratio_margin);

    const double moderate_some = std::max(0.0, config.moderate_psi_some_avg10 - psi_margin);
    const double high_some = std::max(0.0, config.high_psi_some_avg10 - psi_margin);
    const double critical_some = std::max(0.0, config.critical_psi_some_avg10 - psi_margin);

    const double moderate_full = std::max(0.0, config.moderate_psi_full_avg10 - psi_margin);
    const double high_full = std::max(0.0, config.high_psi_full_avg10 - psi_margin);
    const double critical_full = std::max(0.0, config.critical_psi_full_avg10 - psi_margin);

    int result = 0;
    const auto apply_ratio = [&result](double ratio, double moderate, double high, double critical) {
        if (ratio <= critical) result = std::max(result, 3);
        else if (ratio <= high) result = std::max(result, 2);
        else if (ratio <= moderate) result = std::max(result, 1);
    };
    const auto apply_psi = [&result](double value, double moderate, double high, double critical) {
        if (value >= critical) result = std::max(result, 3);
        else if (value >= high) result = std::max(result, 2);
        else if (value >= moderate) result = std::max(result, 1);
    };

    if (signals.available_ratio.has_value()) {
        apply_ratio(*signals.available_ratio, moderate_ratio, high_ratio, critical_ratio);
    }
    if (signals.psi_some.has_value()) {
        apply_psi(*signals.psi_some, moderate_some, high_some, critical_some);
    }
    if (signals.psi_full.has_value()) {
        apply_psi(*signals.psi_full, moderate_full, high_full, critical_full);
    }
    return result;
}

PressureBand band_from_severity(int value) noexcept {
    switch (value) {
        case 0: return PressureBand::normal;
        case 1: return PressureBand::moderate;
        case 2: return PressureBand::high;
        case 3: return PressureBand::critical;
        default: return PressureBand::unknown;
    }
}

bool observation_is_stale(const ObservationBatch& observation,
                          const CoreContext& context,
                          const CoreConfig& config) noexcept {
    if (context.now_ns == 0U) return false;
    if (observation.captured_at_ns == 0U || context.now_ns < observation.captured_at_ns) return true;
    return context.now_ns - observation.captured_at_ns > config.max_observation_age_ns;
}

bool has_safe_file_reclaim_backend(const KernelCapabilities& capabilities) noexcept {
    return capabilities.cgroup_v2 && capabilities.memory_reclaim &&
           capabilities.memory_reclaim_swappiness_zero && capabilities.safe_reclaim_target;
}

} // namespace

CoreEngine::CoreEngine(CoreConfig config)
    : config_(config), config_valid_(validate_core_config(config)) {}

bool CoreEngine::configuration_valid() const noexcept {
    return config_valid_;
}

void CoreEngine::reset() noexcept {
    stable_pressure_ = PressureBand::unknown;
    recovery_samples_seen_ = 0U;
    sequence_ = 0U;
}

CoreDecision CoreEngine::evaluate(const ObservationBatch& observation,
                                  const CoreContext& context) {
    CoreDecision decision{};
    if (sequence_ != std::numeric_limits<std::uint64_t>::max()) ++sequence_;
    decision.sequence = sequence_;

    if (!config_valid_) {
        decision.reason = "invalid CoreConfig; policy evaluation disabled";
        return decision;
    }
    if (observation_is_stale(observation, context, config_)) {
        decision.reason = "observation is stale or has an invalid monotonic timestamp";
        return decision;
    }

    const Signals signals = collect_signals(observation);
    decision.available_ratio = signals.available_ratio;
    decision.psi_some_avg10 = signals.psi_some;
    decision.psi_full_avg10 = signals.psi_full;
    if (!signals.valid) {
        decision.reason = "no valid MemAvailable ratio or memory PSI signal";
        return decision;
    }

    const int raw_severity = classify(signals, config_, false);
    const int current_severity = severity(stable_pressure_);
    if (current_severity < 0) {
        stable_pressure_ = band_from_severity(raw_severity);
        recovery_samples_seen_ = 0U;
    } else if (raw_severity > current_severity) {
        // Escalation is immediate. Recovery is deliberately slower.
        stable_pressure_ = band_from_severity(raw_severity);
        recovery_samples_seen_ = 0U;
    } else if (raw_severity < current_severity) {
        const int recovery_severity = classify(signals, config_, true);
        if (recovery_severity < current_severity) {
            if (recovery_samples_seen_ < config_.recovery_samples) {
                ++recovery_samples_seen_;
            }
            if (recovery_samples_seen_ >= config_.recovery_samples) {
                stable_pressure_ = band_from_severity(current_severity - 1);
                recovery_samples_seen_ = 0U;
            }
        } else {
            recovery_samples_seen_ = 0U;
        }
    } else {
        recovery_samples_seen_ = 0U;
    }

    decision.pressure = stable_pressure_;
    decision.recovery_samples_seen = recovery_samples_seen_;

    if (decision.pressure == PressureBand::critical) {
        decision.action = CoreAction::defer_to_platform;
        decision.reason = "critical pressure: leave emergency handling to Android and the kernel";
        return decision;
    }

    if (decision.pressure == PressureBand::normal) {
        decision.reason = "memory pressure is within the configured operating range";
        return decision;
    }

    if (decision.pressure != PressureBand::moderate &&
        decision.pressure != PressureBand::high) {
        decision.reason = "pressure state is not actionable";
        return decision;
    }

    if (!signals.available_ratio.has_value() ||
        !observation.memory.total_kib.has_value() ||
        !observation.memory.available_kib.has_value()) {
        decision.reason = "pressure detected, but no available-memory ratio exists to size a reclaim request";
        return decision;
    }

    if (!has_safe_file_reclaim_backend(context.capabilities)) {
        decision.reason = "pressure detected; no validated cgroup-v2 file-only reclaim target is available";
        return decision;
    }

    if (!context.reclaimable_file_kib.has_value() ||
        *context.reclaimable_file_kib == 0U) {
        decision.reason = "pressure detected; the platform adapter supplied no reclaimable file-cache estimate";
        return decision;
    }

    const double target_kib_double =
        static_cast<double>(*observation.memory.total_kib) * config_.target_available_ratio;
    const auto target_kib = static_cast<std::uint64_t>(std::ceil(target_kib_double));
    const std::uint64_t available_kib = *observation.memory.available_kib;
    const std::uint64_t deficit_kib = target_kib > available_kib ? target_kib - available_kib : 0U;
    const std::uint64_t request_kib = std::min(
        deficit_kib,
        std::min(config_.max_reclaim_per_cycle_kib, *context.reclaimable_file_kib));

    if (request_kib == 0U) {
        decision.reason = "the recovery target is already met; no reclaim request is needed";
        return decision;
    }

    decision.action = CoreAction::propose_file_cache_reclaim;
    decision.reclaim.mode = ReclaimMode::file_cache_only;
    decision.reclaim.scope = ReclaimScope::adapter_selected_cgroup;
    decision.reclaim.requested_kib = request_kib;
    decision.reclaim.estimated_reclaimable_kib = *context.reclaimable_file_kib;
    decision.reason = "bounded file-cache reclaim proposal; execution remains with a validated platform adapter";
    return decision;
}

const char* to_string(PressureBand value) noexcept {
    switch (value) {
        case PressureBand::normal: return "normal";
        case PressureBand::moderate: return "moderate";
        case PressureBand::high: return "high";
        case PressureBand::critical: return "critical";
        case PressureBand::unknown: return "unknown";
    }
    return "unknown";
}

const char* to_string(CoreAction value) noexcept {
    switch (value) {
        case CoreAction::observe_only: return "observe_only";
        case CoreAction::propose_file_cache_reclaim: return "propose_file_cache_reclaim";
        case CoreAction::defer_to_platform: return "defer_to_platform";
    }
    return "observe_only";
}

const char* to_string(ReclaimMode value) noexcept {
    switch (value) {
        case ReclaimMode::none: return "none";
        case ReclaimMode::file_cache_only: return "file_cache_only";
    }
    return "none";
}

bool validate_core_config(const CoreConfig& config) noexcept {
    const bool ratios_valid =
        finite_between(config.critical_available_ratio, 0.001, 0.99) &&
        finite_between(config.high_available_ratio, 0.002, 0.995) &&
        finite_between(config.moderate_available_ratio, 0.003, 0.999) &&
        config.critical_available_ratio < config.high_available_ratio &&
        config.high_available_ratio < config.moderate_available_ratio;

    const bool some_valid =
        finite_between(config.moderate_psi_some_avg10, 0.0, 100.0) &&
        finite_between(config.high_psi_some_avg10, 0.0, 100.0) &&
        finite_between(config.critical_psi_some_avg10, 0.0, 100.0) &&
        config.moderate_psi_some_avg10 < config.high_psi_some_avg10 &&
        config.high_psi_some_avg10 < config.critical_psi_some_avg10;

    const bool full_valid =
        finite_between(config.moderate_psi_full_avg10, 0.0, 100.0) &&
        finite_between(config.high_psi_full_avg10, 0.0, 100.0) &&
        finite_between(config.critical_psi_full_avg10, 0.0, 100.0) &&
        config.moderate_psi_full_avg10 < config.high_psi_full_avg10 &&
        config.high_psi_full_avg10 < config.critical_psi_full_avg10;

    return ratios_valid && some_valid && full_valid &&
           finite_between(config.recovery_available_margin, 0.0, 0.50) &&
           finite_between(config.recovery_psi_margin, 0.0, 20.0) &&
           config.recovery_samples >= 1U && config.recovery_samples <= 10U &&
           config.max_observation_age_ns > 0U &&
           finite_between(config.target_available_ratio, 0.01, 0.999) &&
           config.max_reclaim_per_cycle_kib > 0U &&
           config.max_reclaim_per_cycle_kib <= 1'048'576ULL;
}

} // namespace aether
