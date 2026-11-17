#include "aether/runtime_agent_abi.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace {

constexpr std::uint32_t kKnownCapabilities =
    AETHER_AGENT_CAP_MEMINFO |
    AETHER_AGENT_CAP_PSI |
    AETHER_AGENT_CAP_VMPRESSURE |
    AETHER_AGENT_CAP_MEMEVENTS |
    AETHER_AGENT_CAP_PIDFD;

constexpr std::uint32_t kKnownCandidateFlags =
    AETHER_CANDIDATE_ELIGIBLE |
    AETHER_CANDIDATE_HAS_RSS |
    AETHER_CANDIDATE_HAS_SWAP |
    AETHER_CANDIDATE_HAS_PIDFD;

constexpr std::uint32_t kMaxPsiMilliPercent = 100000U;

bool valid_candidate(const AetherProcessCandidate& candidate,
                     std::uint32_t capabilities) noexcept {
    if (candidate.token == 0U || candidate.pid <= 0) {
        return false;
    }
    if (candidate.oom_score_adj < -1000 || candidate.oom_score_adj > 1000) {
        return false;
    }
    if ((candidate.flags & ~kKnownCandidateFlags) != 0U ||
        (candidate.flags & AETHER_CANDIDATE_ELIGIBLE) == 0U) {
        return false;
    }
    if ((candidate.flags & AETHER_CANDIDATE_HAS_RSS) == 0U &&
        candidate.rss_kib != 0U) {
        return false;
    }
    if ((candidate.flags & AETHER_CANDIDATE_HAS_SWAP) == 0U &&
        candidate.swap_kib != 0U) {
        return false;
    }
    if ((candidate.flags & AETHER_CANDIDATE_HAS_PIDFD) != 0U &&
        (capabilities & AETHER_AGENT_CAP_PIDFD) == 0U) {
        return false;
    }
    return true;
}

} // namespace

extern "C" int32_t aether_agent_validate_snapshot(
    const AetherAgentSnapshot* snapshot) {
    if (snapshot == nullptr ||
        snapshot->abi_version != AETHER_AGENT_ABI_VERSION ||
        snapshot->struct_size < sizeof(AetherAgentSnapshot) ||
        snapshot->sequence == 0U) {
        return 0;
    }

    if ((snapshot->capabilities & ~kKnownCapabilities) != 0U ||
        snapshot->candidate_count > AETHER_AGENT_MAX_CANDIDATES) {
        return 0;
    }
    if (snapshot->candidate_count != 0U && snapshot->candidates == nullptr) {
        return 0;
    }

    if ((snapshot->capabilities & AETHER_AGENT_CAP_MEMINFO) != 0U) {
        if (snapshot->mem_total_kib == 0U ||
            snapshot->mem_free_kib > snapshot->mem_total_kib ||
            snapshot->mem_available_kib > snapshot->mem_total_kib ||
            snapshot->swap_free_kib > snapshot->swap_total_kib) {
            return 0;
        }
    } else if (snapshot->mem_total_kib != 0U ||
               snapshot->mem_free_kib != 0U ||
               snapshot->mem_available_kib != 0U ||
               snapshot->file_cache_kib != 0U ||
               snapshot->swap_total_kib != 0U ||
               snapshot->swap_free_kib != 0U) {
        return 0;
    }

    if ((snapshot->capabilities & AETHER_AGENT_CAP_PSI) != 0U) {
        if (snapshot->psi_some_avg10_milli_pct > kMaxPsiMilliPercent ||
            snapshot->psi_full_avg10_milli_pct > kMaxPsiMilliPercent) {
            return 0;
        }
    } else if (snapshot->psi_some_avg10_milli_pct != 0U ||
               snapshot->psi_full_avg10_milli_pct != 0U) {
        return 0;
    }

    std::array<std::uint64_t, AETHER_AGENT_MAX_CANDIDATES> tokens{};
    std::array<std::int32_t, AETHER_AGENT_MAX_CANDIDATES> pids{};

    for (std::uint32_t i = 0; i < snapshot->candidate_count; ++i) {
        const auto& candidate = snapshot->candidates[i];
        if (!valid_candidate(candidate, snapshot->capabilities)) {
            return 0;
        }
        tokens[i] = candidate.token;
        pids[i] = candidate.pid;
    }

    const auto token_end = tokens.begin() + snapshot->candidate_count;
    std::sort(tokens.begin(), token_end);
    if (std::adjacent_find(tokens.begin(), token_end) != token_end) {
        return 0;
    }

    const auto pid_end = pids.begin() + snapshot->candidate_count;
    std::sort(pids.begin(), pid_end);
    if (std::adjacent_find(pids.begin(), pid_end) != pid_end) {
        return 0;
    }

    return 1;
}

extern "C" int32_t aether_agent_validate_decision(
    const AetherAgentSnapshot* snapshot,
    const AetherAgentDecision* decision) {
    if (aether_agent_validate_snapshot(snapshot) == 0 ||
        decision == nullptr ||
        decision->abi_version != AETHER_AGENT_ABI_VERSION ||
        decision->struct_size < sizeof(AetherAgentDecision)) {
        return 0;
    }

    if (decision->action == AETHER_AGENT_ACTION_NO_ACTION) {
        return decision->selected_candidate_token == 0U ? 1 : 0;
    }

    if (decision->action != AETHER_AGENT_ACTION_SELECT_CANDIDATE ||
        decision->selected_candidate_token == 0U) {
        return 0;
    }

    for (std::uint32_t i = 0; i < snapshot->candidate_count; ++i) {
        if (snapshot->candidates[i].token ==
            decision->selected_candidate_token) {
            return 1;
        }
    }
    return 0;
}
