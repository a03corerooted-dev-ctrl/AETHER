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
- [x] Fail Safe Gate health contract and heartbeat timeout.
- [x] Bounded event log.
- [x] Supervisor sampling loop and JSON API.
- [x] Loopback-only HTTP server and WebUI.
- [x] ARM64 module packaging scripts.
- [x] Host CI workflow and expanded tests.

## LMKD Runtime Agent — required next milestone
- [ ] Audit target LMKD executable/source/build identity.
- [ ] Define in-process Agent ABI and watchdog/heartbeat integration.
- [ ] Validate ShadowHook primary and Dobby secondary in a disposable process.
- [ ] Validate exclusive authority transfer and LMKD rollback on a device.
- [ ] Run Android/SELinux/root-manager stability tests.

A milestone is complete only when its code and evidence exist. Serenity 0.9
does not claim the unchecked LMKD takeover work is complete.
