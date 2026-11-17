# AETHER hooking backend evaluation

| Candidate | Role | Strengths | Constraints |
|---|---|---|---|
| ShadowHook | Preferred | Android-focused inline hook; ARM64; function hook and instruction interception; symbol/address targeting; unhook API and operation records | Exact function ABI required; target symbols/addresses must be validated; finite trampoline/island resources; exact LMKD binary must be tested |
| Dobby | Secondary | Native C/C++ multi-platform hook framework; Android and ARM64 support | Less Android-specific compatibility guidance in its project overview; exact Android/LMKD behavior requires independent validation |

ShadowHook is the preferred candidate; Dobby is the secondary candidate. These
are candidates for controlled testing, not claims of universal LMKD compatibility.
ShadowHook documents ARM64 function hooks/intercepts, symbol/address targeting,
unhooking, and operation records. Dobby documents Android and ARM64 support in
its multi-platform framework.

## Why inline hooks first
LMKD integration may need to observe internal native paths that do not pass through
a PLT entry. A PLT-only hook would not cover every such path. Inline hooks are the
first candidate class, but require validation of instruction boundaries, function
ABI, relocation/trampoline behavior, and the exact target build.

## Backend rules
- Never install both backends on the same target function.
- Select one backend only after runtime capability checks.
- Keep hook operations disabled in observation-only builds.
- Require validated target identity and expected symbol/code metadata.
- Hook failure, unexpected target bytes, missing symbols, or failed unhook means
  remain in or return to LMKD fallback.
- Do not use hard-coded offsets as cross-device compatibility.

## Validation before integration
1. Identify the exact LMKD executable and build identity on a test device.
2. Inspect symbols and confirm relevant call paths.
3. Test each candidate in a disposable process before LMKD.
4. Measure overhead, thread-safety, and failure behavior.
5. Verify rollback and Fail Safe Gate transition on forced faults.
6. Only then evaluate a non-intervening LMKD observation prototype.

No hook target names or offsets are committed yet. Guessing them would violate
AETHER's runtime-validation requirements.
