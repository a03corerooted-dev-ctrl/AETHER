# AETHER

**AETHER Serenity 0.9.1** is an ARM64 Android memory-management engine developer
preview. Its target architecture remains an in-process Runtime Agent inside the
already-running LMKD process, coordinated by an external Supervisor and WebUI.

## Authority contract
AETHER is the sole memory-policy authority while the Runtime Agent is healthy.
LMKD is preserved only as the emergency fallback through the Fail Safe Gate.
The two must never act as simultaneous policy authorities.

## What Serenity 0.9.1 contains
- C++17 native procfs observer for memory, PSI, and bounded process metadata.
- Deterministic pressure classifier and tested Fail Safe Gate state machine.
- Thread-safe bounded event history.
- Supervisor process with JSON API bound to 127.0.0.1 only.
- Static local WebUI, ARM64 module packaging scripts, and host CI.
- ShadowHook (primary candidate) and Dobby (secondary candidate) documented for
  later target-specific validation.

## Important status
This is an observation/supervisor preview, not yet an LMKD takeover. It does not
install hooks, modify LMKD, kill processes, alter kernel controls, or transfer
policy authority on a device. The Supervisor remains in LMKD_FALLBACK until an
in-process Runtime Agent is implemented and validated. Do not treat the preview
module as a production memory-management release.

## Host build and tests
```sh
./scripts/test-host.sh
```

## Build an ARM64 module package
Set ANDROID_NDK_HOME to an installed Android NDK, then run:
```sh
./scripts/package-module.sh
```
The archive is written to dist/AETHER-Serenity-0.9.1-arm64.zip. Device installation
and operation still require dedicated testing.

See docs/SERENITY_09.md, docs/ARCHITECTURE.md, docs/HOOKING_EVALUATION.md, and
docs/WEBUI_DESIGN.md.
