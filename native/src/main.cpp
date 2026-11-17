#include "aether/supervisor.hpp"
#include "aether/version.hpp"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <sys/stat.h>

namespace {
bool write_snapshot(const std::filesystem::path& directory,
                    const std::string& name, const std::string& contents) {
    const auto target = directory / name;
    const auto temporary = directory / (name + ".tmp");
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) return false;
        output << contents << '\n';
        output.flush();
        if (!output.good()) return false;
    }
    if (::chmod(temporary.c_str(), 0640) != 0) return false;
    std::error_code error;
    std::filesystem::rename(temporary, target, error);
    if (error) {
        std::filesystem::remove(temporary);
        return false;
    }
    return true;
}
bool publish_snapshot(const aether::Supervisor& supervisor,
                      const std::filesystem::path& directory) {
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) return false;
    (void)::chmod(directory.c_str(), 0750);
    return write_snapshot(directory, "status.json", supervisor.status_json()) &&
           write_snapshot(directory, "memory.json", supervisor.memory_json()) &&
           write_snapshot(directory, "processes.json", supervisor.processes_json()) &&
           write_snapshot(directory, "events.json", supervisor.events_json());
}
}

int main(int argc, char** argv) {
    std::string proc_root = "/proc";
    std::filesystem::path state_dir = "/data/adb/modules/aether/state";
    std::uint32_t interval_ms = 1000;
    bool once = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto next = [&]() -> const char* { return (i + 1 < argc) ? argv[++i] : nullptr; };
        if (arg == "--proc-root") {
            const auto value = next();
            if (!value) { std::cerr << "missing --proc-root value\n"; return 2; }
            proc_root = value;
        } else if (arg == "--state-dir") {
            const auto value = next();
            if (!value) { std::cerr << "missing --state-dir value\n"; return 2; }
            state_dir = value;
        } else if (arg == "--interval-ms") {
            const auto value = next();
            if (!value) { std::cerr << "missing --interval-ms value\n"; return 2; }
            const long parsed = std::strtol(value, nullptr, 10);
            if (parsed < 100 || parsed > 60000) {
                std::cerr << "interval must be 100..60000 ms\n";
                return 2;
            }
            interval_ms = static_cast<std::uint32_t>(parsed);
        } else if (arg == "--once") {
            once = true;
        } else if (arg == "--version") {
            std::cout << aether::kVersion << '\n';
            return 0;
        } else {
            std::cerr << "unknown argument: " << arg << '\n';
            return 2;
        }
    }
    aether::Supervisor supervisor(proc_root);
    supervisor.sample_once();
    supervisor.tick();
    if (once) {
        std::cout << supervisor.status_json() << '\n'
                  << supervisor.memory_json() << '\n'
                  << supervisor.processes_json() << '\n'
                  << supervisor.events_json() << '\n';
        return 0;
    }
    if (!publish_snapshot(supervisor, state_dir)) {
        std::cerr << "AETHER: unable to publish initial state snapshot\n";
        return 1;
    }
    std::cout << "AETHER Serenity " << aether::kVersion
              << " publishing manager-readable state in " << state_dir << '\n';
    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        supervisor.sample_once();
        supervisor.tick();
        if (!publish_snapshot(supervisor, state_dir))
            std::cerr << "AETHER: state snapshot update failed\n";
    }
}
