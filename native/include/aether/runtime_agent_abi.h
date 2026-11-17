#ifndef AETHER_RUNTIME_AGENT_ABI_H
#define AETHER_RUNTIME_AGENT_ABI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Stable, bounded C ABI between the AOSP LMKD source adapter and the
 * in-process AETHER Runtime Agent. It deliberately contains no LMKD-private
 * pointers or structure layouts.
 */
#define AETHER_AGENT_ABI_VERSION 1U
#define AETHER_AGENT_MAX_CANDIDATES 1024U

typedef enum AetherAgentCapability {
    AETHER_AGENT_CAP_MEMINFO = 1U << 0,
    AETHER_AGENT_CAP_PSI = 1U << 1,
    AETHER_AGENT_CAP_VMPRESSURE = 1U << 2,
    AETHER_AGENT_CAP_MEMEVENTS = 1U << 3,
    AETHER_AGENT_CAP_PIDFD = 1U << 4
} AetherAgentCapability;

typedef enum AetherCandidateFlag {
    AETHER_CANDIDATE_ELIGIBLE = 1U << 0,
    AETHER_CANDIDATE_HAS_RSS = 1U << 1,
    AETHER_CANDIDATE_HAS_SWAP = 1U << 2,
    AETHER_CANDIDATE_HAS_PIDFD = 1U << 3
} AetherCandidateFlag;

typedef enum AetherAgentAction {
    AETHER_AGENT_ACTION_NO_ACTION = 0,
    AETHER_AGENT_ACTION_SELECT_CANDIDATE = 1
} AetherAgentAction;

typedef struct AetherProcessCandidate {
    uint64_t token;
    int32_t pid;
    uint32_t uid;
    int32_t oom_score_adj;
    uint32_t flags;
    uint64_t rss_kib;
    uint64_t swap_kib;
} AetherProcessCandidate;

typedef struct AetherAgentSnapshot {
    uint32_t abi_version;
    uint32_t struct_size;
    uint64_t sequence;
    uint64_t captured_at_ns;
    uint32_t capabilities;
    uint32_t candidate_count;

    uint64_t mem_total_kib;
    uint64_t mem_free_kib;
    uint64_t mem_available_kib;
    uint64_t file_cache_kib;
    uint64_t swap_total_kib;
    uint64_t swap_free_kib;

    /* PSI avg10 values in milli-percent: 1000 == 1.000 percent. */
    uint32_t psi_some_avg10_milli_pct;
    uint32_t psi_full_avg10_milli_pct;

    /* Points only to candidate_count AetherProcessCandidate values. */
    const AetherProcessCandidate* candidates;
} AetherAgentSnapshot;

typedef struct AetherAgentDecision {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t action;
    uint32_t reason_code;
    uint64_t selected_candidate_token;
} AetherAgentDecision;

/*
 * Returns 1 for a valid ABI object and 0 for invalid input.
 * A failed validation must cause the LMKD adapter to retain/restore fallback.
 */
int32_t aether_agent_validate_snapshot(const AetherAgentSnapshot* snapshot);
int32_t aether_agent_validate_decision(
    const AetherAgentSnapshot* snapshot,
    const AetherAgentDecision* decision);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AETHER_RUNTIME_AGENT_ABI_H */
