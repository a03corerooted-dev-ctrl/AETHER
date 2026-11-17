#include "aether/core_engine.hpp"

#include <cassert>
#include <cstdint>

namespace {
aether::ObservationBatch sample(std::uint64_t total_kib,
                                std::uint64_t available_kib,
                                double psi_some = 0.0,
                                double psi_full = 0.0) {
    aether::ObservationBatch observation{};
    observation.captured_at_ns = 1'000'000'000ULL;
    observation.memory_valid = true;
    observation.memory.total_kib = total_kib;
    observation.memory.available_kib = available_kib;
    observation.pressure_valid = true;
    observation.pressure.some = aether::PressureAverage{psi_some, psi_some, psi_some, 0U};
    observation.pressure.full = aether::PressureAverage{psi_full, psi_full, psi_full, 0U};
    return observation;
}
}

int main() {
    using namespace aether;

    assert(validate_core_config(CoreConfig{}));
    CoreConfig invalid_config{};
    invalid_config.critical_available_ratio = 0.20;
    assert(!validate_core_config(invalid_config));
    CoreEngine invalid_engine(invalid_config);
    assert(!invalid_engine.configuration_valid());

    CoreEngine engine;
    auto normal = engine.evaluate(sample(1'000'000U, 300'000U));
    assert(normal.pressure == PressureBand::normal);
    assert(normal.action == CoreAction::observe_only);

    auto moderate = engine.evaluate(sample(1'000'000U, 140'000U));
    assert(moderate.pressure == PressureBand::moderate);

    auto high = engine.evaluate(sample(1'000'000U, 80'000U));
    assert(high.pressure == PressureBand::high);

    auto critical = engine.evaluate(sample(1'000'000U, 30'000U));
    assert(critical.pressure == PressureBand::critical);
    assert(critical.action == CoreAction::defer_to_platform);

    engine.reset();
    auto psi_pressure = engine.evaluate(sample(1'000'000U, 300'000U, 6.0, 0.0));
    assert(psi_pressure.pressure == PressureBand::high);

    engine.reset();
    ObservationBatch empty{};
    const auto unknown = engine.evaluate(empty);
    assert(unknown.pressure == PressureBand::unknown);
    assert(unknown.action == CoreAction::observe_only);

    engine.reset();
    assert(engine.evaluate(sample(1'000'000U, 30'000U)).pressure == PressureBand::critical);
    const auto recovery_one = engine.evaluate(sample(1'000'000U, 80'000U));
    assert(recovery_one.pressure == PressureBand::critical);
    assert(recovery_one.recovery_samples_seen == 1U);
    const auto recovery_two = engine.evaluate(sample(1'000'000U, 80'000U));
    assert(recovery_two.pressure == PressureBand::high);
    assert(recovery_two.recovery_samples_seen == 0U);

    engine.reset();
    CoreContext safe_context{};
    safe_context.capabilities.cgroup_v2 = true;
    safe_context.capabilities.memory_reclaim = true;
    safe_context.capabilities.memory_reclaim_swappiness_zero = true;
    safe_context.capabilities.safe_reclaim_target = true;
    safe_context.reclaimable_file_kib = 120'000U;
    const auto proposal = engine.evaluate(sample(1'000'000U, 120'000U), safe_context);
    assert(proposal.pressure == PressureBand::moderate);
    assert(proposal.action == CoreAction::propose_file_cache_reclaim);
    assert(proposal.reclaim.mode == ReclaimMode::file_cache_only);
    assert(proposal.reclaim.scope == ReclaimScope::adapter_selected_cgroup);
    assert(proposal.reclaim.requested_kib == 30'000U);

    engine.reset();
    safe_context.capabilities.memory_reclaim_swappiness_zero = false;
    const auto unsupported = engine.evaluate(sample(1'000'000U, 120'000U), safe_context);
    assert(unsupported.action == CoreAction::observe_only);
    assert(unsupported.reclaim.requested_kib == 0U);

    engine.reset();
    CoreContext stale_context{};
    stale_context.now_ns = 10'000'000'000ULL;
    const auto stale = engine.evaluate(sample(1'000'000U, 10'000U), stale_context);
    assert(stale.pressure == PressureBand::unknown);
    assert(stale.action == CoreAction::observe_only);

    return 0;
}
