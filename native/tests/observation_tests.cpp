#include "aether/observation.hpp"
#include <cassert>
#include <string_view>
int main() {
    constexpr std::string_view meminfo =
        "MemTotal: 8192000 kB\nMemFree: 512000 kB\nMemAvailable: 4096000 kB\n"
        "Buffers: 64000 kB\nCached: 1024000 kB\nSwapTotal: 2048000 kB\nSwapFree: 1536000 kB\n";
    const auto memory = aether::parse_meminfo(meminfo);
    assert(memory && memory.value.total_kib == 8192000U);
    assert(memory.value.available_kib == 4096000U && memory.value.swap_free_kib == 1536000U);
    const auto pressure = aether::parse_pressure_line("some avg10=0.25 avg60=0.10 avg300=0.02 total=123456");
    assert(pressure && pressure.value.avg10 > 0.24 && pressure.value.avg10 < 0.26);
    assert(pressure.value.total_us == 123456U);
    const auto missing = aether::parse_meminfo("MemFree: 10 kB\n");
    assert(!missing && missing.error == aether::ParseError::missing_required_field);
    const auto unit = aether::parse_meminfo("MemTotal: 1024 MB\n");
    assert(!unit && unit.error == aether::ParseError::invalid_unit);
    const auto duplicate = aether::parse_meminfo("MemTotal: 1024 kB\nMemTotal: 2048 kB\n");
    assert(!duplicate && duplicate.error == aether::ParseError::duplicate_field);
    const auto full = aether::parse_pressure_line("full avg10=1.00 avg60=0.50 avg300=0.10 total=987654");
    assert(full && full.value.total_us == 987654U);
    const auto impossible = aether::parse_meminfo("MemTotal: 1024 kB\nMemAvailable: 2048 kB\n");
    assert(!impossible && impossible.error == aether::ParseError::invalid_number);
    const auto overflow = aether::parse_meminfo("MemTotal: 18446744073709551616 kB\n");
    assert(!overflow && overflow.error == aether::ParseError::invalid_number);
    const auto malformed = aether::parse_pressure_line("some avg10=nan avg60=0 avg300=0 total=1");
    assert(!malformed);
    return 0;
}
