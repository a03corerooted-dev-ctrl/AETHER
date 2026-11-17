# AETHER architecture

## Non-negotiable authority model
AETHER is the sole policy authority while healthy. It owns all memory-management
policy decisions. The already-running Android LMKD process is the integration
host and the emergency fallback only; it is not a parallel policy authority.

The Fail Safe Gate enforces mutually exclusive authority:
1. **AETHER_ACTIVE** — AETHER has passed health and integrity checks and owns
   policy decisions. The LMKD fallback path is held in reserve.
2. **LMKD_FALLBACK** — AETHER is unavailable, unhealthy, or fails validation. The
   gate relinquishes AETHER authority and allows preserved LMKD behavior to protect
   the system.
3. **TRANSITION** — authority is not established. No new AETHER intervention may
   be issued until transition completes and is verified.

The safe default is \`LMKD_FALLBACK\`. AETHER may become active only after positive
runtime validation. A failed or ambiguous transition resolves to fallback.
Foundation 0.2 specifies this contract but does not yet implement the in-process
gate or alter LMKD behavior.

## Components
### Supervisor
Owns module lifecycle, configuration, health checks, diagnostics, local API, and
future WebUI. It monitors the in-process Runtime Agent and coordinates recovery
without becoming a second policy engine.

### Runtime Agent
Lives inside the existing LMKD process. It is the boundary through which AETHER
observes the target and later assumes policy authority. It validates target
identity, hook results, protocol version, and health before the Fail Safe Gate
grants AETHER authority.

### Policy Core
Consumes normalized memory, pressure, and process observations. It is independent
of private LMKD structures and produces AETHER policy decisions, not suggestions
to a still-active LMKD policy engine.

### WebUI
A local management surface communicating only with the Supervisor API. It cannot
write procfs, kernel controls, or LMKD internals directly.

## Hooking strategy
The first two candidate inline-hook backends are ShadowHook (preferred) and Dobby
(secondary). The project will not load both against the same LMKD target. One
backend is selected per runtime after capability and integrity checks. See
\`HOOKING_EVALUATION.md\`.

No target symbol, offset, private structure layout, or hook point is assumed
universal. AArch64 support does not imply binary compatibility across Android
builds. Unsupported or ambiguous targets remain in fallback mode.

## Safety invariants
- Observe and validate before granting AETHER policy authority.
- Fail closed to LMKD fallback on missing symbols, unknown layouts, failed hooks,
  integrity errors, watchdog timeout, or protocol mismatch.
- Never allow two policy authorities to act simultaneously.
- Do not make blind writes to private process or kernel state.
- Keep a watchdog, kill switch, bounded event queue, and auditable transition log.
- Discover runtime capabilities; do not infer compatibility from OEM names.

## Current implementation: Foundation 0.2
The native core contains normalized observation data types and parsers for Linux
\`/proc/meminfo\` and memory PSI lines. Process fields are optional because sources
may expose different metrics. Host tests cover valid and malformed input. No
Android procfs reader, process enumerator, LMKD Runtime Agent, hook installation,
Fail Safe Gate, policy intervention, Supervisor service, or WebUI is implemented.
