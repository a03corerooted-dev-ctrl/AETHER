#include "aether/runtime_agent_abi.h"

#include <cassert>
#include <cstdint>

namespace {

AetherAgentSnapshot make_snapshot(
    const AetherProcessCandidate* candidates,
    std::uint32_t candidate_count) {
    AetherAgentSnapshot snapshot{};
    snapshot.abi_version = AETHER_AGENT_ABI_VERSION;
    snapshot.struct_size = static_cast<std::uint32_t>(sizeof(snapshot));
    snapshot.sequence = 1U;
    snapshot.captured_at_ns = 123456U;
    snapshot.capabilities = AETHER_AGENT_CAP_MEMINFO | AETHER_AGENT_CAP_PSI |
                            AETHER_AGENT_CAP_PIDFD;
    snapshot.candidate_count = candidate_count;
    snapshot.mem_total_kib = 4096000U;
    snapshot.mem_free_kib = 256000U;
    snapshot.mem_available_kib = 1024000U;
    snapshot.file_cache_kib = 512000U;
    snapshot.swap_total_kib = 1024000U;
    snapshot.swap_free_kib = 768000U;
    snapshot.psi_some_avg10_milli_pct = 1500U;
    snapshot.psi_full_avg10_milli_pct = 200U;
    snapshot.candidates = candidates;
    return snapshot;
}

AetherAgentDecision make_decision(std::uint32_t action,
                                  std::uint64_t candidate_token) {
    AetherAgentDecision decision{};
    decision.abi_version = AETHER_AGENT_ABI_VERSION;
    decision.struct_size = static_cast<std::uint32_t>(sizeof(decision));
    decision.action = action;
    decision.selected_candidate_token = candidate_token;
    return decision;
}

} // namespace

int main() {
    AetherProcessCandidate candidates[2]{};
    candidates[0] = {101U, 123, 10001U, 700,
                     AETHER_CANDIDATE_ELIGIBLE |
                         AETHER_CANDIDATE_HAS_RSS |
                         AETHER_CANDIDATE_HAS_PIDFD,
                     65536U, 0U};
    candidates[1] = {102U, 456, 10002U, 850,
                     AETHER_CANDIDATE_ELIGIBLE |
                         AETHER_CANDIDATE_HAS_RSS |
                         AETHER_CANDIDATE_HAS_SWAP |
                         AETHER_CANDIDATE_HAS_PIDFD,
                     131072U, 4096U};

    auto snapshot = make_snapshot(candidates, 2U);
    assert(aether_agent_validate_snapshot(&snapshot) == 1);

    auto no_action =
        make_decision(AETHER_AGENT_ACTION_NO_ACTION, 0U);
    assert(aether_agent_validate_decision(&snapshot, &no_action) == 1);

    auto select_valid =
        make_decision(AETHER_AGENT_ACTION_SELECT_CANDIDATE, 102U);
    assert(aether_agent_validate_decision(&snapshot, &select_valid) == 1);

    auto select_unknown =
        make_decision(AETHER_AGENT_ACTION_SELECT_CANDIDATE, 999U);
    assert(aether_agent_validate_decision(&snapshot, &select_unknown) == 0);

    auto no_action_with_token =
        make_decision(AETHER_AGENT_ACTION_NO_ACTION, 101U);
    assert(aether_agent_validate_decision(&snapshot, &no_action_with_token) == 0);

    auto bad_version = snapshot;
    bad_version.abi_version++;
    assert(aether_agent_validate_snapshot(&bad_version) == 0);

    auto bad_size = snapshot;
    bad_size.struct_size = 1U;
    assert(aether_agent_validate_snapshot(&bad_size) == 0);

    auto missing_candidate_array = snapshot;
    missing_candidate_array.candidates = nullptr;
    assert(aether_agent_validate_snapshot(&missing_candidate_array) == 0);

    auto too_many = snapshot;
    too_many.candidate_count = AETHER_AGENT_MAX_CANDIDATES + 1U;
    assert(aether_agent_validate_snapshot(&too_many) == 0);

    auto bad_psi = snapshot;
    bad_psi.psi_full_avg10_milli_pct = 100001U;
    assert(aether_agent_validate_snapshot(&bad_psi) == 0);

    auto bad_candidate = snapshot;
    candidates[0].pid = 0;
    assert(aether_agent_validate_snapshot(&bad_candidate) == 0);
    candidates[0].pid = 123;

    auto duplicate_token = snapshot;
    candidates[1].token = candidates[0].token;
    assert(aether_agent_validate_snapshot(&duplicate_token) == 0);
    candidates[1].token = 102U;

    auto duplicate_pid = snapshot;
    candidates[1].pid = candidates[0].pid;
    assert(aether_agent_validate_snapshot(&duplicate_pid) == 0);
    candidates[1].pid = 456;

    auto unsupported_pidfd = snapshot;
    unsupported_pidfd.capabilities &= ~AETHER_AGENT_CAP_PIDFD;
    assert(aether_agent_validate_snapshot(&unsupported_pidfd) == 0);

    auto no_meminfo = snapshot;
    no_meminfo.capabilities &= ~AETHER_AGENT_CAP_MEMINFO;
    no_meminfo.mem_total_kib = 0U;
    no_meminfo.mem_free_kib = 0U;
    no_meminfo.mem_available_kib = 0U;
    no_meminfo.file_cache_kib = 0U;
    no_meminfo.swap_total_kib = 0U;
    no_meminfo.swap_free_kib = 0U;
    assert(aether_agent_validate_snapshot(&no_meminfo) == 1);

    auto no_candidates = make_snapshot(nullptr, 0U);
    assert(aether_agent_validate_snapshot(&no_candidates) == 1);
    auto no_candidates_selection =
        make_decision(AETHER_AGENT_ACTION_SELECT_CANDIDATE, 101U);
    assert(aether_agent_validate_decision(&no_candidates,
                                          &no_candidates_selection) == 0);

    return 0;
}
