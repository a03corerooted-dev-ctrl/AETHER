# AETHER architecture

## Mission

AETHER is an independent Android memory-management engine. It owns the normalized memory model, policy rules, pressure classification, and decision planning. Android and Linux remain the source of truth for process lifecycle, kernel memory accounting, and the execution of supported memory operations.

AETHER is not designed as a replacement launcher, Android framework fork, universal binary patch, or simple LMKD parameter tuner. The core must not depend on a specific OEM, device name, private framework structure, or guessed symbol address.

## Core pipeline

Platform telemetry -> normalized memory model -> pressure intelligence -> policy engine -> reclaim planner -> capability-gated platform adapter

### Memory Model

The normalized model represents system memory, swap, pressure, and bounded per-process accounting. Values are optional because Android kernels and vendor builds expose different subsets of procfs, PSI, cgroup v2, and ZRAM information. An unavailable metric is not synthesized from a different metric.

System memory observations include MemTotal, MemAvailable, free memory, buffers/cache, swap, active/inactive anonymous and file-backed memory, dirty/writeback pages, reclaimable slab, and shared memory when the kernel exports them. Per-process observations currently include RSS, RSS anonymous/file/shared breakdown, VmSwap, UID, process state, and oom_score_adj when readable. PSS and cgroup-level attribution are later adapter work; they must not be approximated from RSS.

### Pressure Intelligence

The first classifier combines the MemAvailable ratio with memory PSI some and full averages. The worst valid signal determines the raw pressure band. Missing PSI does not invalidate a usable MemAvailable signal, and missing MemAvailable does not invalidate usable PSI. If neither is available, the result is unknown.

Pressure escalation is immediate. Recovery requires a configurable number of consecutive samples and a recovery margin, preventing rapid oscillation around a threshold. Stale observations are rejected when the caller supplies a monotonic timestamp.

### Policy Engine

The first Core policy is deliberately conservative:

- normal: observe; no reclaim request.
- moderate / high: consider a bounded, file-cache-only reclaim proposal.
- critical: defer emergency handling to Android and the kernel; AETHER does not kill a process.

Default thresholds are initial policy defaults, not claims of universal optimality. They must be calibrated with measurements and remain configurable.

### Reclaim Planner

The current planner can only describe a possible file-cache reclaim. It emits a plan when all of the following are explicitly supplied by a platform adapter:

1. cgroup v2 is active;
2. the target exposes memory.reclaim;
3. the kernel has been verified to accept the swappiness=0 file-only reclaim form;
4. the adapter has selected an authorized cgroup target;
5. the adapter supplies a bounded estimate of reclaimable file-backed memory.

The requested amount is capped by the deficit to the configured recovery target, the adapter's reclaimable estimate, and a per-cycle maximum. This is an intent only. No kernel write is performed by Core v1.

AETHER must not use a global drop_caches write as a substitute for targeted reclaim. It must not delete application files or application cache directories and must not infer that all cached or inactive memory is immediately reclaimable.

## Platform adapters

Adapters discover capabilities at runtime and expose normalized inputs. The Core must not contain hard-coded Android cgroup paths, OEM process names, or assumptions that all devices run the same kernel configuration.

Planned adapter families:

- procfs: meminfo, process status, vmstat, and PSI when present;
- cgroup v2: per-cgroup memory.current, memory.stat, memory.events, and supported memory.reclaim semantics;
- ZRAM: read-only statistics and device presence;
- Android framework/source integration: process importance and lifecycle metadata through documented or source-built interfaces;
- optional kernel facilities such as DAMON, only when the running kernel exposes and validates them.

Adapters own access checks, path resolution, capability detection, and operation validation. Core owns the decision. An adapter may reject a plan even after Core emits it.

## Coexistence and authority boundaries

AETHER must coexist with existing Android memory management:

- ActivityManager owns app lifecycle and process state.
- Cached App Freezer owns the platform cached-process freeze lifecycle.
- Android 17 Memory Limiter and system memory budgets own their configured per-app and system-process cgroup budgets.
- Kernel reclaim, MGLRU, swap, and ZRAM own page-level memory mechanics.
- LMKD remains the platform's emergency process-kill mechanism unless a separately designed and validated platform integration changes that relationship.

AETHER must not bypass app memory limits, raise or clear app-owned budgets, overwrite critical daemon protection floors, silently unfreeze cached apps, or independently kill processes. The current Core does not own process lifecycle or kill authority.

## Safety and correctness invariants

- Observe before planning; validate every metric and capability.
- Unknown capability means unavailable, never assumed supported.
- No unqualified global cache drop.
- No anonymous-page reclaim in Core v1.
- No process kill, freeze, thaw, trim callback, or app-data deletion in Core v1.
- No writes to Android Memory Limiter or SystemMemoryBudgetManager policy files.
- Every plan is bounded and can be rejected by its adapter.
- All runtime operations require explicit capability checks, permissions, target validation, and rollback behavior.
- Platform fallback remains enabled.

## Current implementation status

The repository has an observation-first Core v1 foundation: normalized meminfo/PSI/process metrics, pressure bands, recovery hysteresis, and a bounded file-cache reclaim planner. The live Supervisor currently supplies telemetry but no validated cgroup reclaim target or capabilities, so it remains in observe_only mode.

There is no active reclaim executor, cgroup writer, Android framework adapter, ZRAM controller, process lifecycle controller, LMKD takeover, or runtime hook installation.
