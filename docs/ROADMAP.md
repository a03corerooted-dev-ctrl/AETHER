# AETHER roadmap

## Foundation 0.1
- [x] CMake project, C++17 baseline, ARM64 Android ABI guard.
- [x] Native lifecycle skeleton and initial architecture.

## Core 0.2
- [x] Normalized memory/process models and memory/PSI parsers.
- [x] Procfs observer, bounded process enumeration, deterministic fixtures.
- [x] Host tests for parser and observer behavior.

## Serenity 0.9 developer preview
- [x] Memory-pressure policy classification.
- [x] Host-testable Fail Safe Gate state machine and heartbeat timeout.
- [x] Bounded event log.
- [x] Supervisor sampling loop and JSON snapshots.
- [x] ARM64 module packaging and host CI workflow.
- [x] Manager-hosted WebUI and Magisk action diagnostic.
- [x] Removal of the embedded HTTP server and localhost dependency.

## AOSP LMKD source integration — next milestone
- [ ] Pin and document the AOSP LMKD source baseline and supported source API.
- [ ] Define a versioned, bounded, C-compatible Runtime Agent ABI.
- [ ] Implement an AOSP source adapter using the official LMKD lifecycle hooks
      where appropriate.
- [ ] Add a source-level policy-decision bridge before the original LMKD
      victim-selection path.
- [ ] Ensure AETHER decisions are exclusive while active and the original AOSP
      policy path is restored on any failed validation or Agent health fault.
- [ ] Delegate process execution, pidfd lifecycle, kill accounting, and
      reporting to LMKD's existing reaper path.
- [ ] Add deterministic tests for activation, fallback, stale candidates,
      deadline expiry, property refresh, and rollback.
- [ ] Build the integrated LMKD target through the AOSP Soong build.

## Runtime prebuilt-binary research — non-canonical
- [ ] Evaluate whether a separately supported prebuilt deployment mode is
      technically viable without OEM-specific assumptions.
- [ ] Keep ShadowHook and Dobby disabled unless target identity, hook ABI,
      integrity, and rollback are positively validated.

A milestone is complete only when its code and evidence exist. Serenity 0.9.2
does not claim the in-process Runtime Agent or LMKD takeover is complete.
