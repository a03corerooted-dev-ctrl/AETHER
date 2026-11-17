#include "aether/engine.hpp"

namespace aether {

bool Engine::start_observation() noexcept {
    EngineState expected = EngineState::stopped;
    if (!state_.compare_exchange_strong(
            expected,
            EngineState::observing,
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        return false;
    }

    transition_count_.fetch_add(1, std::memory_order_relaxed);
    return true;
}

bool Engine::stop() noexcept {
    EngineState current = state_.load(std::memory_order_acquire);

    while (current != EngineState::stopped) {
        if (state_.compare_exchange_weak(
                current,
                EngineState::stopped,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
            transition_count_.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
    }

    return false;
}

EngineSnapshot Engine::snapshot() const noexcept {
    return EngineSnapshot{
        state_.load(std::memory_order_acquire),
        transition_count_.load(std::memory_order_relaxed),
    };
}

} // namespace aether
