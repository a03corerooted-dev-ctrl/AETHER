# AETHER

**AETHER** is an ARM64-first Android memory-management engine project.

The long-term design places an AETHER policy engine alongside the existing Android
Low Memory Killer Daemon (LMKD), with a native runtime agent, an external supervisor,
and a management WebUI. AETHER is intended to make policy decisions; Android's
existing memory-management mechanisms remain responsible for kernel-facing actions.

> **Development status:** Foundation 0.1. This repository currently contains the
> initial native core and architecture contracts. It does not yet inject into,
> hook, replace, or control a running LMKD process. Do not install this foundation
> as a functional memory-management module.

## Project constraints

- Target architecture: **AArch64 / arm64-v8a only** for Android builds.
- Native implementation: C++17.
- Build system: CMake + Android NDK.
- OEM-independent design goal; runtime capabilities must be detected and validated,
  never assumed from a device brand or Android version.
- Fail closed: unsupported or unverified runtime integration must not activate
  intervention.

## Repository layout

- `native/` — native core and public C++ interfaces.
- `docs/` — architecture, build instructions, and implementation roadmap.
- `config/` — example configuration schema.
- `module/` — initial packaging metadata only; not an installable engine yet.

## Build the native foundation on a host

```sh
cmake -S . -B build -DAETHER_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Configure an Android ARM64 build

```sh
cmake -S . -B build-android \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-23 \
  -DAETHER_BUILD_TESTS=OFF
cmake --build build-android --parallel
```

The Android API level above is a build baseline, not a compatibility guarantee for
private LMKD internals. Runtime integration requires separate device-side validation.

## Current status

See [Architecture](docs/ARCHITECTURE.md), [Build guide](docs/BUILDING.md), and
[Roadmap](docs/ROADMAP.md).

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).
