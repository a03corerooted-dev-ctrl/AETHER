#pragma once
#include "aether/procfs_reader.hpp"
#include <cstdint>
#include <string>

namespace aether {
enum class PressureLevel : std::uint8_t { normal=0, elevated=1, critical=2, unknown=3 };
enum class PolicyAction : std::uint8_t { no_action=0, prepare_reclaim=1, emergency_pressure=2, hold_fallback=3 };
struct PolicyDecision final {
    PressureLevel level{PressureLevel::unknown};
    PolicyAction action{PolicyAction::hold_fallback};
    double available_ratio{0.0};
    std::string reason{"no valid memory observation"};
    std::uint64_t source_timestamp_ns{0};
};
class MemoryPolicy final {
public:
    [[nodiscard]] PolicyDecision evaluate(const ObservationBatch& observation) const;
};
[[nodiscard]] const char* to_string(PressureLevel value) noexcept;
[[nodiscard]] const char* to_string(PolicyAction value) noexcept;
} // namespace aether
