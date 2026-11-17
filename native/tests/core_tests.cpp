#include "aether/engine.hpp"

#include <cassert>

int main() {
    aether::Engine engine;

    const auto initial = engine.snapshot();
    assert(initial.state == aether::EngineState::stopped);
    assert(initial.transition_count == 0U);

    assert(engine.start_observation());
    const auto observing = engine.snapshot();
    assert(observing.state == aether::EngineState::observing);
    assert(observing.transition_count == 1U);

    // Starting an already-running engine must not create another transition.
    assert(!engine.start_observation());
    assert(engine.snapshot().transition_count == 1U);

    assert(engine.stop());
    const auto stopped = engine.snapshot();
    assert(stopped.state == aether::EngineState::stopped);
    assert(stopped.transition_count == 2U);

    // Stopping an already-stopped engine is an idempotent no-op.
    assert(!engine.stop());
    assert(engine.snapshot().transition_count == 2U);

    return 0;
}
