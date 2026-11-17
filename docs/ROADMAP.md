# AETHER roadmap

## Foundation 0.1 — repository baseline

- [x] CMake project and C++17 configuration.
- [x] Android ABI guard for arm64-v8a.
- [x] Minimal native lifecycle core.
- [x] Host-side lifecycle tests.
- [x] Initial architecture and build documentation.
- [ ] Run CI on a clean host toolchain and publish verified results.

## Core 0.2 — observation model

- [ ] Define normalized memory-pressure and process observation types.
- [ ] Add Linux/Android data-source interfaces with explicit capability reporting.
- [ ] Add bounded event transport and structured diagnostics.
- [ ] Add configuration parsing and validation.
- [ ] Add policy unit tests using deterministic fixtures.

## Runtime 0.3 — LMKD integration research

- [ ] Inventory supported AOSP LMKD source/build variants.
- [ ] Define a versioned runtime-agent ABI.
- [ ] Prototype read-only discovery and integrity checks on dedicated test devices.
- [ ] Document SELinux and process-boundary constraints.
- [ ] Keep intervention disabled until the read-only path is reliable.

## Supervisor and WebUI

- [ ] Supervisor lifecycle, watchdog, and kill switch.
- [ ] Local authenticated API and explicit permission model.
- [ ] WebUI dashboard, process view, policy configuration, and event history.

## Intervention milestone

Intervention is a separate gated milestone. It requires repeatable device tests,
measurable policy benefit, bounded failure behavior, and a verified recovery path.
No roadmap item should be treated as implemented until code, tests, and evidence
are committed.
