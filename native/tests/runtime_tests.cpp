#include "aether/event_log.hpp"
#include <utility>
#include "aether/failsafe_gate.hpp"
#include "aether/policy.hpp"
#include "aether/procfs_reader.hpp"
#include "aether/supervisor.hpp"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

namespace {
void write(const std::filesystem::path& path,const std::string& data) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path); out<<data; assert(out.good());
}
}
int main() {
    const auto root=std::filesystem::temp_directory_path()/"aether-serenity-fixture";
    std::filesystem::remove_all(root);
    write(root/"meminfo","MemTotal: 1000000 kB\nMemFree: 50000 kB\nMemAvailable: 200000 kB\nCached: 1000 kB\nSwapTotal: 200000 kB\nSwapFree: 150000 kB\n");
    write(root/"pressure"/"memory","some avg10=0.50 avg60=0.20 avg300=0.10 total=20\nfull avg10=0.00 avg60=0.00 avg300=0.00 total=0\n");
    write(root/"123"/"status","Name:\ttest.proc\nState:\tS (sleeping)\nUid:\t10001 10001 10001 10001\nVmRSS:\t4096 kB\n");
    write(root/"123"/"oom_score_adj","200\n");

    aether::ProcfsReader reader(root.string(),32);
    const auto batch=reader.collect();
    assert(batch.memory_valid);
    assert(batch.pressure_valid);
    assert(batch.processes.size()==1U);
    assert(batch.processes[0].pid==123);
    assert(batch.processes[0].uid==10001U);
    assert(batch.processes[0].rss_kib==4096U);
    assert(batch.processes[0].oom_score_adj==200);

    aether::MemoryPolicy policy;
    const auto decision=policy.evaluate(batch);
    assert(decision.level==aether::PressureLevel::normal);

    auto critical=batch;
    critical.memory.available_kib=10000U;
    const auto critical_decision=policy.evaluate(critical);
    assert(critical_decision.level==aether::PressureLevel::critical);
    assert(critical_decision.action==aether::PolicyAction::emergency_pressure);

    aether::FailSafeGate gate(std::chrono::milliseconds(15));
    assert(gate.snapshot().state==aether::AuthorityState::lmkd_fallback);
    aether::AgentHealth invalid{true,true,true,false,true};
    assert(!gate.activate(invalid));
    assert(gate.snapshot().state==aether::AuthorityState::lmkd_fallback);
    aether::AgentHealth valid{true,true,true,true,true};
    assert(gate.activate(valid));
    assert(gate.snapshot().state==aether::AuthorityState::aether_active);
    gate.heartbeat();
    assert(!gate.poll());
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    assert(gate.poll());
    assert(gate.snapshot().state==aether::AuthorityState::lmkd_fallback);

    aether::EventLog log(2);
    log.push("INFO","test","one"); log.push("INFO","test","two"); log.push("INFO","test","three");
    const auto events=log.snapshot(10);
    assert(events.size()==2U && events.front().message=="two");

    aether::Supervisor supervisor(root.string());
    supervisor.sample_once();
    assert(supervisor.status_json().find("LMKD_FALLBACK")!=std::string::npos);
    assert(supervisor.memory_json().find("\"total_kib\":1000000")!=std::string::npos);
    assert(supervisor.processes_json().find("test.proc")!=std::string::npos);
    assert(supervisor.events().size()>=2U);
    std::filesystem::remove_all(root);
    return 0;
}
