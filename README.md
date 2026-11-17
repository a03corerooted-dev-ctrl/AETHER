# AETHER

**AETHER** is an OEM-independent Android ARM64 memory-management engine. It is being developed as an independent C++17 core with a normalized memory model, pressure intelligence, policy evaluation, and bounded reclaim planning.

AETHER is not a theme, launcher, RAM cleaner, or a wrapper around LMKD. Its core owns its own memory-policy model while using Android and Linux interfaces as telemetry and execution mechanisms. It must coexist with platform services and kernel reclaim rather than bypass them.

## Core architecture

- **Memory Model:** normalized system and per-process memory observations.
- **Pressure Intelligence:** combines MemAvailable and memory PSI signals with explicit handling for missing or stale metrics.
- **Policy Engine:** configurable pressure bands, escalation, hysteresis, and recovery.
- **Reclaim Planner:** creates bounded, capability-gated reclaim intents. The planner does not write kernel controls.
- **Platform Adapters:** discover supported procfs, PSI, cgroup v2, ZRAM, and optional reclaim facilities.
- **Supervisor:** sampling, state publication, diagnostics, and health reporting.

The design is ARM64-only for Android packaging, but the policy core is kept independent of OEM names, device profiles, private framework structures, and guessed binary offsets.

## Current implementation

The repository currently includes:

- C++17 native core and ARM64 NDK packaging.
- /proc/meminfo and memory PSI parsing.
- Bounded /proc process enumeration with RSS, anonymous/file/shmem RSS, swap, UID, state, and oom_score_adj where readable.
- AETHER Core pressure bands (normal, moderate, high, critical) with immediate escalation and sample-based hysteresis on recovery.
- A file-cache-only reclaim planner that emits a bounded proposal only when a future platform adapter explicitly confirms cgroup v2, memory.reclaim, swappiness=0 support, an authorized target, and a reclaimable estimate.
- Host-side tests and ARM64 build CI.
- Manager-hosted static WebUI snapshots. The WebUI remains observational.

## Explicit non-goals for the current milestone

AETHER does **not** currently write to /proc, /sys, or cgroup controls; change app memory limits; clear application data/cache; freeze processes; kill processes; reconfigure ZRAM; or replace LMKD/ActivityManager. No reclaim proposal is executed. Missing kernel capabilities remain unavailable rather than being guessed.

Android platform services such as Memory Limiter, SystemMemoryBudgetManager, Cached App Freezer, and LMKD remain authoritative for the functions they own. AETHER must not bypass Android app memory budgets or critical-process protection.

## Build and test

Host build and tests:

    ./scripts/test-host.sh

Build an Android ARM64 module with Android NDK r27.2 or compatible:

    ./scripts/package-module.sh

The module archive is written to dist/AETHER-Serenity-0.9.2-arm64.zip.

## Research references

- docs/ARCHITECTURE.md — engine architecture and invariants.
- docs/PLATFORM_LIMITS.md — Android/kernel capabilities, constraints, and compatibility boundaries.
- docs/ROADMAP.md — implementation milestones.
- docs/AOSP_INTEGRATION.md and docs/HOOKING_EVALUATION.md — optional integration research, not Core dependencies.
- docs/WEBUI_DESIGN.md — manager-hosted WebUI model.
