# AETHER

**AETHER** is an ARM64-only Android memory-management engine project. Its intended
runtime integration is inside the already-running LMKD process, using a native
Runtime Agent coordinated by an external Supervisor and managed through a WebUI.

## Authority model
AETHER is the sole owner of memory-management policy decisions while healthy.
LMKD is preserved only as the emergency fallback, controlled by the Fail Safe
Gate. AETHER and LMKD must never act as simultaneous policy authorities. See
[Architecture](docs/ARCHITECTURE.md).

## Development status
Foundation 0.2 adds normalized memory/process observation types and tested parsers
for \`/proc/meminfo\` and memory PSI lines. It remains observation-only: it does not
yet read Android procfs, enumerate live processes, integrate with LMKD, install
hooks, implement the Fail Safe Gate, intervene in policy, or provide the
Supervisor/WebUI runtime. Do not install it as a functioning memory-management
module.

## Fixed constraints
- Android ABI: **AArch64 / arm64-v8a only**.
- Native implementation: C++17, CMake, Android NDK.
- Universal/OEM-independent design goal; runtime capability checks are mandatory.
- Existing LMKD process is the integration host, not a replacement target.
- AETHER owns policy decisions; LMKD is fallback only.
- Fail closed to LMKD fallback on any uncertain runtime state.

## Build and test
\`\`\`sh
cmake -S . -B build -DAETHER_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
\`\`\`

ShadowHook is the preferred hook candidate and Dobby is the secondary candidate.
Neither is integrated yet; the exact LMKD binary and call paths must be audited.
See [Hooking evaluation](docs/HOOKING_EVALUATION.md) and [WebUI design](docs/WEBUI_DESIGN.md).
