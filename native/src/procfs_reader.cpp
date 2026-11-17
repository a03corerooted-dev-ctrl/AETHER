#include "aether/procfs_reader.hpp"
#include <utility>
#include <algorithm>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>
#include <system_error>

namespace aether {
namespace {
std::uint64_t monotonic_ns() {
    const auto now=std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());
}
std::string trim(std::string value) {
    const auto first=value.find_first_not_of(" \t\r\n");
    if(first==std::string::npos) return {};
    const auto last=value.find_last_not_of(" \t\r\n");
    return value.substr(first,last-first+1U);
}
bool parse_kib_value(const std::string& value, std::uint64_t& output) {
    std::istringstream in(value);
    std::uint64_t parsed = 0;
    std::string unit;
    if (!(in >> parsed >> unit) || unit != "kB") return false;
    output = parsed;
    return true;
}
bool read_process(const std::filesystem::path& dir,ProcessObservation& p) {
    std::ifstream status(dir/"status");
    if(!status) return false;
    std::string line;
    while(std::getline(status,line)) {
        const auto colon=line.find(':');
        if(colon==std::string::npos) continue;
        const auto key=std::string_view(line).substr(0,colon);
        const auto value=trim(line.substr(colon+1U));
        std::uint64_t amount = 0;
        if(key=="Name") p.name=value;
        else if(key=="State" && !value.empty()) p.state=value.front();
        else if(key=="Uid") {
            std::istringstream in(value);
            std::uint64_t uid=0;
            if(in>>uid && uid<=UINT32_MAX) p.uid=static_cast<std::uint32_t>(uid);
        } else if(key=="VmRSS" && parse_kib_value(value, amount)) p.rss_kib=amount;
        else if(key=="RssAnon" && parse_kib_value(value, amount)) p.rss_anon_kib=amount;
        else if(key=="RssFile" && parse_kib_value(value, amount)) p.rss_file_kib=amount;
        else if(key=="RssShmem" && parse_kib_value(value, amount)) p.rss_shmem_kib=amount;
        else if(key=="VmSwap" && parse_kib_value(value, amount)) p.swap_kib=amount;
    }
    std::ifstream adj(dir/"oom_score_adj");
    std::string adj_text;
    if(adj>>adj_text) {
        std::int32_t score=0;
        const auto parsed=std::from_chars(adj_text.data(),adj_text.data()+adj_text.size(),score);
        if(parsed.ec==std::errc{} && parsed.ptr==adj_text.data()+adj_text.size() && score>=-1000 && score<=1000)
            p.oom_score_adj=score;
    }
    return !p.name.empty();
}
} // namespace

ProcfsReader::ProcfsReader(std::string proc_root,std::size_t max_processes)
    :root_(std::move(proc_root)),max_processes_(std::max<std::size_t>(1U,max_processes)) {}

ObservationBatch ProcfsReader::collect() const {
    ObservationBatch out{};
    out.captured_at_ns=monotonic_ns();

    std::ifstream meminfo(std::filesystem::path(root_)/"meminfo");
    if(meminfo) {
        std::ostringstream contents;
        contents<<meminfo.rdbuf();
        const auto parsed=parse_meminfo(contents.str());
        if(parsed) { out.memory=parsed.value; out.memory.monotonic_timestamp_ns=out.captured_at_ns; out.memory_valid=true; }
        else out.source_error="meminfo parse failed";
    } else out.source_error="meminfo unavailable";

    std::ifstream psi(std::filesystem::path(root_)/"pressure"/"memory");
    if(psi) {
        std::string line;
        bool valid=true;
        while(std::getline(psi,line)) {
            std::istringstream tokens(line);
            std::string kind;
            tokens>>kind;
            const auto parsed=parse_pressure_line(line);
            if(!parsed) { valid=false; continue; }
            if(kind=="some") out.pressure.some=parsed.value;
            else if(kind=="full") out.pressure.full=parsed.value;
        }
        out.pressure_valid=valid && (out.pressure.some.has_value() || out.pressure.full.has_value());
    }

    std::error_code ec;
    const std::filesystem::path proc(root_);
    std::vector<std::pair<std::int32_t,std::filesystem::path>> candidates;
    for(std::filesystem::directory_iterator it(proc,std::filesystem::directory_options::skip_permission_denied,ec),end;
        !ec && it!=end && candidates.size()<max_processes_*4U;it.increment(ec)) {
        if(!it->is_directory(ec)) continue;
        const auto name=it->path().filename().string();
        std::int32_t pid=0;
        const auto parsed=std::from_chars(name.data(),name.data()+name.size(),pid);
        if(parsed.ec==std::errc{} && parsed.ptr==name.data()+name.size() && pid>0)
            candidates.emplace_back(pid,it->path());
    }
    std::sort(candidates.begin(),candidates.end(),[](const auto& a,const auto& b){return a.first<b.first;});
    for(const auto& item:candidates) {
        if(out.processes.size()>=max_processes_) break;
        ProcessObservation process{};
        process.pid=item.first;
        if(read_process(item.second,process)) out.processes.push_back(std::move(process));
    }
    return out;
}
} // namespace aether
