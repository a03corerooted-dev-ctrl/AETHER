# AETHER roadmap

## Foundation / Serenity preview
- [x] CMake project, C++17 baseline, ARM64 Android ABI guard.
- [x] Static module packaging and manager-hosted WebUI.
- [x] Procfs observer, bounded process enumeration, and memory PSI parser.
- [x] Host CI, deterministic fixtures, and runtime snapshot smoke tests.

## AETHER Core v1 — independent policy foundation
- [x] Add a standalone CoreEngine independent of LMKD-private structures.
- [x] Normalize additional system memory fields: active/inactive anon/file, dirty, writeback, reclaimable slab, and shmem.
- [x] Extend process observations with anonymous/file/shared RSS and VmSwap where exported.
- [x] Combine MemAvailable and PSI into normal/moderate/high/critical pressure bands.
- [x] Implement immediate pressure escalation and sample-based recovery hysteresis.
- [x] Reject stale observations when a monotonic caller timestamp is supplied.
- [x] Add a bounded file-cache-only reclaim planner with explicit cgroup-v2/kernel capability gates.
- [x] Keep the live Supervisor in observe-only mode; no kernel operation is executed.
- [x] Add deterministic CoreEngine tests and strict-warning CI coverage.

## Telemetry adapters
- [ ] Add feature-detected vmstat and cgroup-v2 accounting.
- [ ] Add read-only ZRAM statistics and compression/swap efficiency metrics.
- [ ] Add process PSS and cgroup attribution with sampling budgets; do not read smaps_rollup for every process on every tick.
- [ ] Add an Android process-importance adapter using a source-defined, versioned contract.
- [ ] Make telemetry freshness and per-source errors visible in diagnostics.

## Memory intelligence and planning
- [ ] Add page-cache and anonymous-memory working-set estimates from validated kernel/cgroup counters.
- [ ] Add reclaim efficiency tracking: requested vs actually reclaimed, stall cost, CPU cost, and refault signals.
- [ ] Add adaptive per-device policy calibration based on measured telemetry, not OEM-name profiles.
- [ ] Add staged reclaim policies with rate limits, cooldowns, quotas, and rollback.
- [ ] Evaluate optional DAMON support only when the kernel provides it.

## Execution backends
- [ ] Implement a read-only-first cgroup adapter and explicit target authorization.
- [ ] Execute file-only reclaim only after target and kernel semantics are validated.
- [ ] Add bounded reclaim result verification and automatic cooldown on ineffective or harmful operations.
- [ ] Do not bypass Android app budgets, critical service protections, or platform process lifecycle.

## Android integration
- [ ] Define a stable Android source adapter independent of the Core ABI.
- [ ] Integrate with ActivityManager/process lifecycle only through documented or source-built Android interfaces.
- [ ] Keep LMKD and platform emergency recovery as fallback; no parallel kill authority.
- [ ] Keep runtime binary hooks experimental and disabled unless each target ABI is validated.

Milestones are complete only when code, tests, device evidence, and rollback behavior exist. A host test does not prove that a kernel interface is present or safe on every Android device.
