# Building AETHER Serenity 0.9.2

## Host build and test

Requirements: CMake 3.22+, C++17 compiler, and a POSIX-compatible host.

    ./scripts/test-host.sh

The host binary reads /proc by default. A deterministic fixture can be
supplied with --proc-root PATH. --once prints a single status, memory,
process, and event snapshot and exits. --version prints the version string.
The long-running Supervisor publishes atomic JSON snapshots to --state-dir
PATH (default: /data/adb/modules/aether/state).

## Android ARM64 package

Install Android NDK r27.2 or compatible and set ANDROID_NDK_HOME.

    ./scripts/package-module.sh

The package contains the manager-hosted static UI at webroot/index.html.
It is written to dist/AETHER-Serenity-0.9.2-arm64.zip.

## Manager compatibility

KernelSU and KernelSU Next can host the module WebUI and expose the ksu
JavaScript bridge. Magisk exposes the module action.sh action console; it
does not host the same integrated WebUI. The module never starts an HTTP
server or binds a local/network port.

## Hard boundary

This is not an LMKD takeover build. It does not install ShadowHook or Dobby,
inject into LMKD, alter LMKD policy, kill processes, or write kernel memory
controls. LMKD remains unmodified and in service. A production takeover
requires an audited in-process Runtime Agent and device-tested Fail Safe Gate handoff.


The example policy file is a reference schema for the Core policy values. Runtime JSON configuration loading is not wired into the current Supervisor yet; the active host defaults are constructed through CoreConfig in native code.
