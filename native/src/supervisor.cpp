#include "aether/supervisor.hpp"
#include <utility>
#include <iomanip>
#include <sstream>

namespace aether {
std::string json_escape(const std::string& value) {
    std::ostringstream out;
    for (char raw : value) {
        const auto c = static_cast<unsigned char>(raw);
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (c < 0x20U) {
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<unsigned>(c) << std::dec;
                } else {
                    out << static_cast<char>(c);
                }
        }
    }
    return out.str();
}

Supervisor::Supervisor(std::string root) : reader_(std::move(root)), events_(256) {
    events_.push("INFO", "supervisor", "Serenity 0.9 supervisor initialized; LMKD fallback retained");
}
void Supervisor::sample_once() {
    auto fresh = reader_.collect();
    auto next_decision = policy_.evaluate(fresh);
    const bool memory_valid = fresh.memory_valid;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        observation_ = std::move(fresh);
        decision_ = std::move(next_decision);
        last_error_ = memory_valid ? std::string{} : observation_.source_error;
    }
    if (!memory_valid) events_.push("WARN", "observer", "memory source unavailable or invalid");
    else events_.push("DEBUG", "observer", "memory and process observation refreshed");
}
void Supervisor::tick() {
    if (gate_.poll()) events_.push("ERROR", "failsafe", "agent heartbeat expired; LMKD fallback selected");
}
ObservationBatch Supervisor::observation() const { std::lock_guard<std::mutex> lock(mutex_); return observation_; }
PolicyDecision Supervisor::decision() const { std::lock_guard<std::mutex> lock(mutex_); return decision_; }
GateSnapshot Supervisor::gate() const { return gate_.snapshot(); }
std::vector<Event> Supervisor::events(std::size_t limit) const { return events_.snapshot(limit); }

std::string Supervisor::status_json() const {
    const auto g=gate(); const auto d=decision(); const auto o=observation();
    std::ostringstream s;
    s << "{\"version\":\"0.9.0-serenity\",\"authority\":\"" << to_string(g.state)
      << "\",\"gate_generation\":" << g.generation << ",\"gate_reason\":\"" << json_escape(g.reason)
      << "\",\"memory_valid\":" << (o.memory_valid?"true":"false") << ",\"process_count\":" << o.processes.size()
      << ",\"pressure_level\":\"" << to_string(d.level) << "\",\"policy_action\":\"" << to_string(d.action) << "\"}";
    return s.str();
}
std::string Supervisor::memory_json() const {
    const auto o=observation(); const auto d=decision();
    std::ostringstream s;
    s << "{\"valid\":" << (o.memory_valid?"true":"false") << ",\"captured_at_ns\":" << o.captured_at_ns
      << ",\"total_kib\":" << (o.memory.total_kib?std::to_string(*o.memory.total_kib):"null")
      << ",\"available_kib\":" << (o.memory.available_kib?std::to_string(*o.memory.available_kib):"null")
      << ",\"free_kib\":" << (o.memory.free_kib?std::to_string(*o.memory.free_kib):"null")
      << ",\"cached_kib\":" << (o.memory.cached_kib?std::to_string(*o.memory.cached_kib):"null")
      << ",\"swap_total_kib\":" << (o.memory.swap_total_kib?std::to_string(*o.memory.swap_total_kib):"null")
      << ",\"swap_free_kib\":" << (o.memory.swap_free_kib?std::to_string(*o.memory.swap_free_kib):"null")
      << ",\"psi_some_avg10\":" << (o.pressure.some?std::to_string(o.pressure.some->avg10):"null")
      << ",\"psi_full_avg10\":" << (o.pressure.full?std::to_string(o.pressure.full->avg10):"null")
      << ",\"pressure_level\":\"" << to_string(d.level) << "\",\"policy_action\":\"" << to_string(d.action) << "\"}";
    return s.str();
}
std::string Supervisor::processes_json() const {
    const auto o=observation(); std::ostringstream s;
    s << "{\"count\":" << o.processes.size() << ",\"processes\":[";
    bool first=true;
    for(const auto& p:o.processes) {
        if(!first) s << ',';
        first=false;
        s << "{\"pid\":" << p.pid << ",\"uid\":" << (p.uid?std::to_string(*p.uid):"null")
          << ",\"name\":\"" << json_escape(p.name) << "\",\"rss_kib\":" << (p.rss_kib?std::to_string(*p.rss_kib):"null")
          << ",\"oom_score_adj\":" << (p.oom_score_adj?std::to_string(*p.oom_score_adj):"null")
          << ",\"state\":\"" << (p.state?std::string(1,*p.state):"") << "\"}";
    }
    s << "]}"; return s.str();
}
std::string Supervisor::events_json() const {
    const auto list=events(); std::ostringstream s; s << "{\"events\":[";
    bool first=true;
    for(const auto& e:list) {
        if(!first) s << ',';
        first=false;
        s << "{\"sequence\":" << e.sequence << ",\"timestamp_ms\":" << e.timestamp_ms
          << ",\"level\":\"" << json_escape(e.level) << "\",\"category\":\"" << json_escape(e.category)
          << "\",\"message\":\"" << json_escape(e.message) << "\"}";
    }
    s << "]}"; return s.str();
}
} // namespace aether
