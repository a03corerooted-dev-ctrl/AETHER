#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
namespace aether {
struct MemorySnapshot final {
    std::uint64_t monotonic_timestamp_ns{0};
    std::optional<std::uint64_t> total_kib, available_kib, free_kib, buffers_kib, cached_kib, swap_total_kib, swap_free_kib;
};
struct PressureAverage final {
    double avg10{0.0}, avg60{0.0}, avg300{0.0};
    std::uint64_t total_us{0};
};
struct MemoryPressureSnapshot final {
    std::optional<PressureAverage> some, full;
};
struct ProcessObservation final {
    std::int32_t pid{0};
    std::optional<std::uint32_t> uid;
    std::string name;
    std::optional<std::uint64_t> rss_kib;
    std::optional<std::int32_t> oom_score_adj;
    std::optional<char> state;
};
enum class ParseError : std::uint8_t { none=0, missing_required_field, invalid_number, invalid_unit, duplicate_field, invalid_pressure_line };
struct MemoryParseResult final {
    MemorySnapshot value;
    ParseError error{ParseError::none};
    [[nodiscard]] explicit operator bool() const noexcept { return error == ParseError::none; }
};
struct PressureParseResult final {
    PressureAverage value;
    ParseError error{ParseError::none};
    [[nodiscard]] explicit operator bool() const noexcept { return error == ParseError::none; }
};
[[nodiscard]] MemoryParseResult parse_meminfo(std::string_view input);
[[nodiscard]] PressureParseResult parse_pressure_line(std::string_view input);
} // namespace aether
