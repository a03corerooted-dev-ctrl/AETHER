#pragma once
#include "aether/event_log.hpp"
#include "aether/failsafe_gate.hpp"
#include "aether/policy.hpp"
#include <mutex>
namespace aether {
class Supervisor final {
public:
    explicit Supervisor(std::string proc_root="/proc");
    void sample_once();
    void tick();
    [[nodiscard]] ObservationBatch observation() const;
    [[nodiscard]] PolicyDecision decision() const;
    [[nodiscard]] GateSnapshot gate() const;
    [[nodiscard]] std::vector<Event> events(std::size_t limit=64) const;
    [[nodiscard]] std::string status_json() const;
    [[nodiscard]] std::string memory_json() const;
    [[nodiscard]] std::string processes_json() const;
    [[nodiscard]] std::string events_json() const;
private:
    ProcfsReader reader_;
    MemoryPolicy policy_;
    FailSafeGate gate_;
    EventLog events_;
    mutable std::mutex mutex_;
    ObservationBatch observation_;
    PolicyDecision decision_;
    std::string last_error_;
};
[[nodiscard]] std::string json_escape(const std::string& value);
} // namespace aether
