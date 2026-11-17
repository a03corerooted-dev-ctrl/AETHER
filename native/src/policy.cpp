#include "aether/policy.hpp"
namespace aether {
const char* to_string(PressureLevel v) noexcept {
    switch(v){case PressureLevel::normal:return "normal";case PressureLevel::elevated:return "elevated";case PressureLevel::critical:return "critical";default:return "unknown";}
}
const char* to_string(PolicyAction v) noexcept {
    switch(v){case PolicyAction::no_action:return "no_action";case PolicyAction::prepare_reclaim:return "prepare_reclaim";case PolicyAction::emergency_pressure:return "emergency_pressure";default:return "hold_fallback";}
}
PolicyDecision MemoryPolicy::evaluate(const ObservationBatch& o) const {
    PolicyDecision d{};
    d.source_timestamp_ns=o.captured_at_ns;
    if(!o.memory_valid || !o.memory.total_kib || *o.memory.total_kib==0U || !o.memory.available_kib) return d;
    d.available_ratio=static_cast<double>(*o.memory.available_kib)/static_cast<double>(*o.memory.total_kib);
    const double some=o.pressure.some?o.pressure.some->avg10:0.0;
    const double full=o.pressure.full?o.pressure.full->avg10:0.0;
    const bool psi_valid=o.pressure.some.has_value() || o.pressure.full.has_value();
    if(d.available_ratio<=0.05 || (psi_valid && (some>=10.0 || full>=5.0))) {
        d.level=PressureLevel::critical; d.action=PolicyAction::emergency_pressure;
        d.reason="critical available-memory ratio or PSI pressure";
    } else if(d.available_ratio<=0.12 || (psi_valid && (some>=2.0 || full>=1.0))) {
        d.level=PressureLevel::elevated; d.action=PolicyAction::prepare_reclaim;
        d.reason="elevated available-memory ratio or PSI pressure";
    } else {
        d.level=PressureLevel::normal; d.action=PolicyAction::no_action;
        d.reason="memory pressure below configured thresholds";
    }
    return d;
}
} // namespace aether
