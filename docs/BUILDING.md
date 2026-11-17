# Building AETHER Serenity 0.9

## Host build and test

Requirements: CMake 3.22+, C++17 compiler, and POSIX-compatible host for the
HTTP server and procfs fixture tests.

```sh
./scripts/test-host.sh
```

The host binary reads the host's `/proc` by default. A deterministic fixture can
be supplied with `--proc-root PATH`; `--once` prints a single status and memory
sample and exits. `--version` prints the version string.

## Android ARM64 cross-build

Install Android NDK r27.2 or compatible and set `ANDROID_NDK_HOME`.

```sh
./scripts/package-module.sh
```

The script cross-compiles `aether-supervisor` for `arm64-v8a`, stages the
Magisk/KernelSU-style module tree, and creates
`dist/AETHER-Serenity-0.9.0-arm64.zip`.

## Runtime

The module service waits for Android boot completion, then starts the read-only
observer. The HTTP server binds exclusively to `127.0.0.1:8765`. The WebUI is
served at the local root path; JSON endpoints are under `/api/v1/`.

## Hard boundary

This is not an LMKD takeover build. It does not install ShadowHook or Dobby,
inject into LMKD, alter LMKD policy, kill processes, or write kernel memory
controls. LMKD remains unmodified and in service. A production takeover requires
an audited in-process Runtime Agent and device-tested Fail Safe Gate handoff.
