#pragma once

#include <atomic>
#include <cstdint>

namespace aether {

enum class EngineState : std::uint8_t {
    stopped = 0,
    ready = 1,
    observing = 2,
    degraded = 3,
};

struct EngineSnapshot final {
    EngineState state;
    std::uint64_t transition_count;
};

// Foundation lifecycle only. This class does not attach to or modify LMKD.
class Engine final {
public:
    Engine() noexcept = default;

    [[nodiscard]] bool start_observation() noexcept;
    [[nodiscard]] bool stop() noexcept;
    [[nodiscard]] EngineSnapshot snapshot() const noexcept;

private:
    std::atomic<EngineState> state_{EngineState::stopped};
    std::atomic<std::uint64_t> transition_count_{0};
};

} // namespace aether
