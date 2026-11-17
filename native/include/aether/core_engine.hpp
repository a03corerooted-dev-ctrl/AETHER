#pragma once

#include "aether/procfs_reader.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace aether {

enum class PressureBand : std::uint8_t {
    normal = 0,
    moderate = 1,
    high = 2,
    critical = 3,
    unknown = 255,
};

enum class CoreAction : std::uint8_t {
    observe_only = 0,
    propose_file_cache_reclaim = 1,
    defer_to_platform = 2,
};

enum class ReclaimMode : std::uint8_t {
    none = 0,
    file_cache_only = 1,
};

enum class ReclaimScope : std::uint8_t {
    none = 0,
    adapter_selected_cgroup = 1,
};

struct KernelCapabilities final {
    bool psi{false};
    bool cgroup_v2{false};
    bool memory_reclaim{false};
    bool memory_reclaim_swappiness_zero{false};
    bool safe_reclaim_target{false};
};

struct CoreContext final {
    KernelCapabilities capabilities{};
    std::optional<std::uint64_t> reclaimable_file_kib;
    // Supply a CLOCK_MONOTONIC timestamp to enforce freshness. Zero disables
    // the age check, which is useful for deterministic offline evaluation.
    std::uint64_t now_ns{0};
};

struct CoreConfig final {
    double moderate_available_ratio{0.15};
    double high_available_ratio{0.10};
    double critical_available_ratio{0.05};

    double moderate_psi_some_avg10{1.0};
    double high_psi_some_avg10{5.0};
    double critical_psi_some_avg10{10.0};

    double moderate_psi_full_avg10{0.10};
    double high_psi_full_avg10{1.0};
    double critical_psi_full_avg10{5.0};

    double recovery_available_margin{0.02};
    double recovery_psi_margin{0.50};
    std::uint32_t recovery_samples{2};

    std::uint64_t max_observation_age_ns{5'000'000'000ULL};
    double target_available_ratio{0.15};
    std::uint64_t max_reclaim_per_cycle_kib{65'536ULL};
};

struct ReclaimPlan final {
    ReclaimMode mode{ReclaimMode::none};
    ReclaimScope scope{ReclaimScope::none};
    std::uint64_t requested_kib{0};
    std::uint64_t estimated_reclaimable_kib{0};
};

struct CoreDecision final {
    PressureBand pressure{PressureBand::unknown};
    CoreAction action{CoreAction::observe_only};
    std::optional<double> available_ratio;
    std::optional<double> psi_some_avg10;
    std::optional<double> psi_full_avg10;
    std::uint32_t recovery_samples_seen{0};
    std::uint64_t sequence{0};
    ReclaimPlan reclaim{};
    std::string reason{"no valid memory or PSI observation"};
};

class CoreEngine final {
public:
    explicit CoreEngine(CoreConfig config = {});

    [[nodiscard]] bool configuration_valid() const noexcept;
    [[nodiscard]] CoreDecision evaluate(
        const ObservationBatch& observation,
        const CoreContext& context = {});
    void reset() noexcept;

private:
    CoreConfig config_;
    bool config_valid_{false};
    PressureBand stable_pressure_{PressureBand::unknown};
    std::uint32_t recovery_samples_seen_{0};
    std::uint64_t sequence_{0};
};

[[nodiscard]] const char* to_string(PressureBand value) noexcept;
[[nodiscard]] const char* to_string(CoreAction value) noexcept;
[[nodiscard]] const char* to_string(ReclaimMode value) noexcept;
[[nodiscard]] bool validate_core_config(const CoreConfig& config) noexcept;

} // namespace aether
