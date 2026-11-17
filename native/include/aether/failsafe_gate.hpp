#pragma once
#include <chrono>
#include <cstdint>
#include <mutex>
namespace aether {
enum class AuthorityState : std::uint8_t { lmkd_fallback=0, transition=1, aether_active=2 };
struct AgentHealth final {
    bool agent_present{false};
    bool integrity_valid{false};
    bool policy_ready{false};
    bool hook_backend_ready{false};
    bool watchdog_ready{false};
};
struct GateSnapshot final {
    AuthorityState state{AuthorityState::lmkd_fallback};
    std::uint64_t generation{0};
    std::uint64_t last_heartbeat_ms{0};
    const char* reason{"startup: LMKD fallback retained"};
};
class FailSafeGate final {
public:
    explicit FailSafeGate(std::chrono::milliseconds heartbeat_timeout=std::chrono::milliseconds(3000));
    [[nodiscard]] bool activate(const AgentHealth& health);
    void heartbeat();
    [[nodiscard]] bool poll();
    void force_fallback(const char* reason) noexcept;
    [[nodiscard]] GateSnapshot snapshot() const;
private:
    mutable std::mutex mutex_;
    std::chrono::milliseconds timeout_;
    AuthorityState state_{AuthorityState::lmkd_fallback};
    std::chrono::steady_clock::time_point last_heartbeat_{};
    std::uint64_t generation_{0};
    const char* reason_{"startup: LMKD fallback retained"};
};
[[nodiscard]] const char* to_string(AuthorityState value) noexcept;
} // namespace aether
