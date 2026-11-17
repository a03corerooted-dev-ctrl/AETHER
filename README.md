# AETHER

**AETHER Serenity 0.9.2** is an ARM64 Android memory-observer developer preview.
Its intended integration target is the existing Android LMKD service, with an
in-process Runtime Agent, an external Supervisor, and a manager-hosted WebUI.

## AOSP-first integration

The canonical source target is AOSP `platform/system/memory/lmkd`. AETHER is
designed to integrate into the already-running `lmkd` process at source/build
time, rather than launch a replacement daemon or maintain OEM-specific offset
tables. The Runtime Agent and the existing LMKD service must remain one process.

AOSP already defines optional compile-time hooks in `include/lmkd_hooks.h`:
initialization, property refresh, pre-kill memory reclamation, and the
no-kill-candidates callback. These are useful lifecycle extension points, but
they do not provide a complete policy-authority handoff. In particular, the
pre-kill hook can only report that memory was freed elsewhere and allow LMKD to
skip that kill. AETHER therefore needs an additional source-level decision
bridge before LMKD's normal victim-selection path.

The official AOSP hooks are enabled at build time through Soong configuration
and a linked `liblmkdhooks`; they are not a runtime plugin interface. The
AOSP-source integration is the canonical universal path. ShadowHook and Dobby
remain optional runtime-hook research candidates, not a basis for claiming
compatibility with arbitrary prebuilt LMKD binaries.

## WebUI architecture

The WebUI is a static module page stored at `module/webroot/index.html` and
loaded by a compatible module manager inside its own WebView. AETHER does not
start an HTTP server, bind a socket, open a localhost port, or require an
external browser. The page calls the manager-provided `ksu.exec` bridge to read
root-owned JSON snapshots published by the Supervisor.

KernelSU and KernelSU Next provide a module WebUI host and JavaScript bridge.
Magisk provides an `action.sh` button/console, not an equivalent integrated
module WebUI host. AETHER's Magisk action prints a one-shot diagnostic
snapshot; it is not presented as a WebUI.

## Authority contract

AETHER is the sole memory-policy authority while the Runtime Agent is healthy.
The existing LMKD process remains the execution and emergency fallback host.
When AETHER is active, LMKD's normal policy-selection path must be bypassed;
when AETHER is unhealthy or validation fails, the original AOSP policy path is
restored. The two policy engines must never make concurrent decisions.

## What Serenity 0.9.2 contains

- C++17 native procfs observer for memory, PSI, and bounded process metadata.
- Memory-pressure classifier and a tested Fail Safe Gate state machine.
- Thread-safe bounded event history.
- Supervisor that atomically publishes status, memory, process, and event snapshots.
- Static manager-hosted WebUI and Magisk `action.sh` diagnostic.
- ARM64 module packaging scripts and CI checks rejecting HTTP-host dependencies.

## Important status

This is an observation/Supervisor preview, not an LMKD takeover. The Runtime
Agent is not yet linked into AOSP LMKD, the decision bridge is not implemented,
and policy authority is not transferred on a device. The existing Fail Safe
Gate is currently a tested core state machine, not a gate controlling the real
LMKD decision path.

A source-built AOSP integration can avoid OEM-specific symbol offsets and
binary layouts by compiling against the AOSP LMKD source interface. A module
installed on top of an arbitrary prebuilt Android image cannot gain a
compile-time AOSP hook merely by placing a library in the filesystem. That
deployment path needs a separately validated runtime integration mechanism and
must not be described as universally compatible without evidence.

## Host build and test

    ./scripts/test-host.sh

## Build ARM64 module

Set `ANDROID_NDK_HOME` to an installed Android NDK, then run:

    ./scripts/package-module.sh

The archive is written to `dist/AETHER-Serenity-0.9.2-arm64.zip`.

See `docs/AOSP_INTEGRATION.md`, `docs/WEBUI_DESIGN.md`,
`docs/BUILDING.md`, `docs/ARCHITECTURE.md`, and
`docs/HOOKING_EVALUATION.md`.
