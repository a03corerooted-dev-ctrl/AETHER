#include "aether/observation.hpp"
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>
namespace aether {
namespace {
bool parse_u64(std::string_view text, std::uint64_t& out) noexcept {
    if (text.empty()) return false;
    std::uint64_t value = 0;
    for (const char ch : text) {
        if (ch < '0' || ch > '9') return false;
        const auto digit = static_cast<std::uint64_t>(ch - '0');
        if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10U) return false;
        value = value * 10U + digit;
    }
    out = value;
    return true;
}
void trim_left(std::string_view& value) noexcept {
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) value.remove_prefix(1);
}
std::string_view take_token(std::string_view& value) noexcept {
    trim_left(value);
    const auto end = value.find_first_of(" \t\r\n");
    const auto token = value.substr(0, end);
    if (end == std::string_view::npos) value = {};
    else value.remove_prefix(end);
    return token;
}
bool parse_double(std::string_view text, double& out) noexcept {
    if (text.empty()) return false;
    const std::string copy(text);
    char* end = nullptr;
    errno = 0;
    const double value = std::strtod(copy.c_str(), &end);
    if (errno != 0 || end == copy.c_str() || *end != '\0' || !std::isfinite(value) || value < 0.0 || value > 100.0) return false;
    out = value;
    return true;
}
} // namespace
MemoryParseResult parse_meminfo(std::string_view input) {
    MemoryParseResult result{};
    std::size_t offset = 0;
    while (offset < input.size()) {
        const auto line_end = input.find('\n', offset);
        auto line = input.substr(offset, line_end == std::string_view::npos ? input.size() - offset : line_end - offset);
        offset = line_end == std::string_view::npos ? input.size() : line_end + 1U;
        if (line.empty()) continue;
        const auto colon = line.find(':');
        if (colon == std::string_view::npos) continue;
        const auto key = line.substr(0, colon);
        auto rest = line.substr(colon + 1U);
        const auto number_token = take_token(rest);
        if (number_token.empty()) continue;
        const bool known = key == "MemTotal" || key == "MemAvailable" || key == "MemFree" || key == "Buffers" ||
                           key == "Cached" || key == "SwapTotal" || key == "SwapFree" ||
                           key == "Active(anon)" || key == "Inactive(anon)" || key == "Active(file)" ||
                           key == "Inactive(file)" || key == "Dirty" || key == "Writeback" ||
                           key == "SReclaimable" || key == "Shmem";
        std::uint64_t number = 0;
        if (!parse_u64(number_token, number)) {
            if (known) { result.error = ParseError::invalid_number; return result; }
            continue;
        }
        if (!known) continue;
        if (take_token(rest) != "kB") { result.error = ParseError::invalid_unit; return result; }
        auto assign = [&](std::optional<std::uint64_t>& field) {
            if (field.has_value()) return false;
            field = number;
            return true;
        };
        bool assigned = false;
        if (key == "MemTotal") assigned = assign(result.value.total_kib);
        else if (key == "MemAvailable") assigned = assign(result.value.available_kib);
        else if (key == "MemFree") assigned = assign(result.value.free_kib);
        else if (key == "Buffers") assigned = assign(result.value.buffers_kib);
        else if (key == "Cached") assigned = assign(result.value.cached_kib);
        else if (key == "SwapTotal") assigned = assign(result.value.swap_total_kib);
        else if (key == "SwapFree") assigned = assign(result.value.swap_free_kib);
        else if (key == "Active(anon)") assigned = assign(result.value.active_anon_kib);
        else if (key == "Inactive(anon)") assigned = assign(result.value.inactive_anon_kib);
        else if (key == "Active(file)") assigned = assign(result.value.active_file_kib);
        else if (key == "Inactive(file)") assigned = assign(result.value.inactive_file_kib);
        else if (key == "Dirty") assigned = assign(result.value.dirty_kib);
        else if (key == "Writeback") assigned = assign(result.value.writeback_kib);
        else if (key == "SReclaimable") assigned = assign(result.value.slab_reclaimable_kib);
        else if (key == "Shmem") assigned = assign(result.value.shmem_kib);
        if (!assigned) { result.error = ParseError::duplicate_field; return result; }
    }
    if (!result.value.total_kib || *result.value.total_kib == 0U) {
        result.error = ParseError::missing_required_field;
        return result;
    }
    if (result.value.available_kib && *result.value.available_kib > *result.value.total_kib) {
        result.error = ParseError::invalid_number;
        return result;
    }
    return result;
}
PressureParseResult parse_pressure_line(std::string_view input) {
    PressureParseResult result{};
    bool seen10=false, seen60=false, seen300=false, seenTotal=false;
    while (!input.empty()) {
        const auto token = take_token(input);
        if (token.empty()) break;
        const auto equal = token.find('=');
        if (equal == std::string_view::npos) continue;
        const auto key = token.substr(0, equal);
        const auto value = token.substr(equal + 1U);
        if (key == "avg10" || key == "avg60" || key == "avg300") {
            double parsed = 0.0;
            if (!parse_double(value, parsed)) { result.error = ParseError::invalid_pressure_line; return result; }
            bool* seen = key == "avg10" ? &seen10 : (key == "avg60" ? &seen60 : &seen300);
            if (*seen) { result.error = ParseError::duplicate_field; return result; }
            *seen = true;
            if (key == "avg10") result.value.avg10 = parsed;
            else if (key == "avg60") result.value.avg60 = parsed;
            else result.value.avg300 = parsed;
        } else if (key == "total") {
            if (seenTotal || !parse_u64(value, result.value.total_us)) {
                result.error = ParseError::invalid_pressure_line;
                return result;
            }
            seenTotal = true;
        }
    }
    if (!seen10 || !seen60 || !seen300 || !seenTotal) result.error = ParseError::invalid_pressure_line;
    return result;
}
} // namespace aether
