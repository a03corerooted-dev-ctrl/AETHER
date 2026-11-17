# Optional LMKD hooking research

> **Architecture status:** Runtime hooks are not part of AETHER Core v1 and are not needed by the independent memory model, policy engine, or reclaim planner. This document records an optional future integration study only. Hooking remains disabled, target-specific, and non-universal.

## If LMKD integration is later pursued: AOSP source-level path

The canonical integration target is the AOSP source project
`platform/system/memory/lmkd`. This avoids treating OEM-specific binary
addresses, symbol tables, and private structure layouts as a universal API.

The current upstream AOSP source contains
`include/lmkd_hooks.h`. Its optional callbacks are:

| Callback | AOSP role | AETHER use / limitation |
|---|---|---|
| `lmkd_update_props_hook()` | Called when LMKD properties are loaded or refreshed | Agent configuration refresh and validation |
| `lmkd_init_hook()` | Called at the end of LMKD initialization | Agent bootstrap and capability handshake |
| `lmkd_free_memory_before_kill_hook(...)` | May reclaim memory before a selected kill; positive returned page count skips that kill | Reclamation integration only; not a policy-selection override |
| `lmkd_no_kill_candidates_hook()` | Called when LMKD has no more candidates at any priority | Last-resort notification; not a policy-selection override |

These hooks are compile-time opt-in. AOSP's Soong configuration defines
`LMKD_USE_HOOKS` and links `liblmkdhooks` when the relevant configuration is
enabled. Without that build configuration, the header supplies no-op inline
definitions. This is a source/build integration point, not a runtime plugin
loader.

### Possible future AETHER extension

The existing hook surface is not sufficient for AETHER's exclusive-authority
contract. AETHER needs a dedicated source-level decision bridge before the
normal LMKD victim-selection path:

- Pass a versioned, normalized snapshot of memory pressure and eligible
  process candidates to the in-process Agent.
- Return a bounded decision: no action, or a selected candidate identified by
  validated process identity fields.
- When the Agent owns authority, bypass the original LMKD policy-selection
  algorithm for that decision cycle.
- Revalidate the candidate in LMKD and use LMKD's existing reaper, pidfd,
  accounting, and event-reporting path for execution.
- If the Agent is unhealthy, returns an invalid/stale decision, or exceeds the
  decision deadline, restore the original AOSP policy path before another
  policy action can occur.

The bridge must not expose raw pointers or private LMKD structure layouts as
the stable Agent ABI. A thin adapter inside the AOSP source tree may translate
between source-native objects and versioned AETHER POD structures.

## AOSP behavior used as the source of truth

The upstream LMKD implementation uses an event-driven loop and kernel-backed
memory-pressure signals, including PSI/vmpressure and memory-event facilities
where supported. It tracks process priority/candidate data and uses pidfd and
its reaper for process lifecycle handling. AETHER's policy model must be
derived from these upstream inputs and explicitly feature-detect unavailable
capabilities rather than assuming every kernel has every feature.

The AOSP `lmkd.rc` service definition confirms that LMKD is an init-managed
core service with its own service identity, capabilities, and control socket.
AETHER's source adapter belongs in that process; starting a second policy
daemon would violate the architecture.

## Secondary research: runtime inline hooks

| Candidate | Role | Constraints |
|---|---|---|
| ShadowHook | First runtime-hook candidate for controlled research | Requires a validated target ABI and exact hook target; not a universal source API |
| Dobby | Second runtime-hook candidate | Requires independent target-build validation; not a universal source API |

Runtime inline hooks are not the canonical AOSP integration. They may be
considered only for a separately defined prebuilt-binary deployment mode.
Unknown targets, ambiguous code identity, failed hook installation, or failed
rollback must remain in `LMKD_FALLBACK`. Never install both backends against
the same target function.

## Validation sequence

1. Pin the AOSP source revision used for integration and record its upstream
   commit.
2. Define a versioned C-compatible Agent ABI with bounded input/output data.
3. Implement the AOSP source adapter and the single-owner decision bridge.
4. Build the existing LMKD target with the Agent linked into the same process.
5. Test fallback, activation, stale candidate rejection, watchdog expiry,
   property refresh, and rollback in deterministic host tests.
6. Build and run AOSP integration tests with the normal LMKD reaper path.
7. Validate an integrated system image before making any runtime takeover
   claim.

No runtime hook target names or offsets are committed. Guessing them would
violate AETHER's source-first and fail-closed requirements.
