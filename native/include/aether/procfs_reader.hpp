#pragma once
#include "aether/observation.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace aether {
struct ObservationBatch final {
    std::uint64_t captured_at_ns{0};
    MemorySnapshot memory;
    MemoryPressureSnapshot pressure;
    std::vector<ProcessObservation> processes;
    bool memory_valid{false};
    bool pressure_valid{false};
    std::string source_error;
};

class ProcfsReader final {
public:
    explicit ProcfsReader(std::string proc_root="/proc", std::size_t max_processes=128);
    [[nodiscard]] ObservationBatch collect() const;
private:
    std::string root_;
    std::size_t max_processes_;
};
} // namespace aether
