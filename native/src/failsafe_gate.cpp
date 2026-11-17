#include "aether/failsafe_gate.hpp"
namespace aether {
FailSafeGate::FailSafeGate(std::chrono::milliseconds timeout):timeout_(timeout){}
bool FailSafeGate::activate(const AgentHealth& h) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_=AuthorityState::transition; ++generation_;
    if(!(h.agent_present&&h.integrity_valid&&h.policy_ready&&h.hook_backend_ready&&h.watchdog_ready)) {
        state_=AuthorityState::lmkd_fallback; reason_="activation rejected: agent health contract incomplete"; return false;
    }
    last_heartbeat_=std::chrono::steady_clock::now();
    state_=AuthorityState::aether_active; reason_="agent health contract validated"; return true;
}
void FailSafeGate::heartbeat() {
    std::lock_guard<std::mutex> lock(mutex_);
    if(state_==AuthorityState::aether_active) last_heartbeat_=std::chrono::steady_clock::now();
}
bool FailSafeGate::poll() {
    std::lock_guard<std::mutex> lock(mutex_);
    if(state_!=AuthorityState::aether_active) return false;
    if(std::chrono::steady_clock::now()-last_heartbeat_>timeout_) {
        state_=AuthorityState::lmkd_fallback; ++generation_; reason_="agent heartbeat timeout: LMKD fallback restored"; return true;
    }
    return false;
}
void FailSafeGate::force_fallback(const char* reason) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    if(state_!=AuthorityState::lmkd_fallback) ++generation_;
    state_=AuthorityState::lmkd_fallback; reason_=reason?reason:"forced fallback";
}
GateSnapshot FailSafeGate::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::uint64_t ms=0;
    if(last_heartbeat_.time_since_epoch().count()!=0)
      ms=static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(last_heartbeat_.time_since_epoch()).count());
    return {state_,generation_,ms,reason_};
}
const char* to_string(AuthorityState v) noexcept {
    switch(v){case AuthorityState::lmkd_fallback:return "LMKD_FALLBACK";case AuthorityState::transition:return "TRANSITION";case AuthorityState::aether_active:return "AETHER_ACTIVE";}
    return "LMKD_FALLBACK";
}
} // namespace aether
