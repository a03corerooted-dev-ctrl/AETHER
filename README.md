# AETHER

**AETHER Serenity 0.9.2** is an ARM64 Android memory-observer developer preview.
Its target architecture remains an in-process Runtime Agent inside the already-running
LMKD process, coordinated by an external Supervisor and a manager-hosted WebUI.

## WebUI architecture

The WebUI is a static module page stored at webroot/index.html and loaded by a
compatible module manager inside its own WebView. AETHER does not start an HTTP
server, bind a socket, open a localhost port, or require an external browser.
The page calls the manager-provided ksu.exec bridge to read root-owned JSON
snapshots published by the Supervisor.

KernelSU and KernelSU Next provide a module WebUI host and JavaScript bridge.
Magisk provides an action.sh button/console, not an equivalent integrated
module WebUI host. AETHER's Magisk action prints a one-shot diagnostic snapshot;
it is not presented as a WebUI.

## Authority contract

AETHER is the sole memory-policy authority while the Runtime Agent is healthy.
LMKD is preserved only as the emergency fallback through the Fail Safe Gate.
The two must never act as simultaneous policy authorities.

## What Serenity 0.9.2 contains

- C++17 native procfs observer for memory, PSI, and bounded process metadata.
- Deterministic pressure classifier and tested Fail Safe Gate state machine.
- Thread-safe bounded event history.
- Supervisor that atomically publishes status, memory, process, and event snapshots.
- Static manager-hosted webroot dashboard and Magisk action.sh diagnostic.
- ARM64 module packaging scripts and CI checks rejecting HTTP-host dependencies.

## Important status

This is an observation/supervisor preview, not an LMKD takeover. It does not
install hooks, modify LMKD, kill processes, alter kernel controls, or transfer
policy authority on a device. The Supervisor remains in LMKD_FALLBACK until an
in-process Runtime Agent is implemented and validated. Device installation and
manager-specific WebView validation are still required.

## Host build and test

    ./scripts/test-host.sh

## Build ARM64 module

Set ANDROID_NDK_HOME to an installed Android NDK, then run:

    ./scripts/package-module.sh

The archive is written to dist/AETHER-Serenity-0.9.2-arm64.zip.

See docs/WEBUI_DESIGN.md, docs/BUILDING.md, docs/ARCHITECTURE.md, and
docs/HOOKING_EVALUATION.md.
