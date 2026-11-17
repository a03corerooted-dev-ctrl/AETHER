# AETHER architecture

## Non-negotiable authority model

AETHER is the sole policy authority while its in-process Runtime Agent is
healthy. The already-running Android LMKD process is the integration host and
the emergency fallback only; it is not a parallel policy authority.

The Fail Safe Gate enforces mutually exclusive authority:

1. **AETHER_ACTIVE** — the in-process Agent has passed health and integrity
   checks and owns policy decisions. LMKD's normal victim-selection path is
   bypassed; the LMKD process remains responsible for validated execution and
   accounting.
2. **LMKD_FALLBACK** — AETHER is unavailable, unhealthy, or fails validation.
   The gate relinquishes AETHER authority and restores the original AOSP LMKD
   policy path.
3. **TRANSITION** — authority is not established. Neither policy path may issue
   a new intervention until the transition is verified.

The safe default is `LMKD_FALLBACK`. AETHER may become active only after
positive Runtime Agent validation. A failed or ambiguous transition resolves
to fallback. The Serenity 0.9.2 gate currently implements a host-testable state
machine; it is not yet connected to LMKD's real decision path.

## Canonical integration target: AOSP LMKD

The canonical source target is AOSP
`platform/system/memory/lmkd`. AETHER must integrate into the existing
`lmkd` service process, not create a second daemon and not replace the service
with an unrelated process.

The Runtime Agent is intended to be compiled and linked into the existing
LMKD target through an AOSP source/build adapter. The integration must use
source-level semantic interfaces and normalized AETHER-owned data structures,
not OEM names, device model checks, guessed symbol addresses, or hard-coded
binary offsets.

AOSP's `include/lmkd_hooks.h` currently exposes optional lifecycle and
reclamation hooks: `lmkd_update_props_hook`, `lmkd_init_hook`,
`lmkd_free_memory_before_kill_hook`, and
`lmkd_no_kill_candidates_hook`. The hooks are compile-time opt-in through
Soong configuration and a linked `liblmkdhooks`; the default definitions are
no-ops. They do not by themselves transfer policy authority. The pre-kill
callback can report memory reclaimed elsewhere so that a particular kill is
skipped, but it does not replace LMKD's candidate selection.

AETHER therefore requires an additional source-level policy bridge at the
decision boundary before the normal AOSP victim-selection path. The bridge
must provide a stable, versioned, normalized ABI to the Agent. While
`AETHER_ACTIVE`, AETHER chooses whether and which eligible process should be
reaped. LMKD must revalidate the selected process using its own current
identity/lifetime data and retain its existing reaper, pidfd, kill accounting,
and event-reporting mechanisms. If the Agent fails or returns an invalid or
stale decision, the gate must restore the unmodified AOSP policy path.

The current AOSP `lmkd_hooks.h` is a build-time extension interface, not a
runtime shared-library plugin contract. A source-built integration is the
canonical universal path. A module-only integration against arbitrary
prebuilt LMKD binaries is a separate problem and cannot inherit compile-time
hooks that were not linked into the target binary.

## Components

### Runtime Agent

Lives inside the existing LMKD process. It validates the source adapter,
protocol version, policy readiness, and health state. It consumes normalized
memory-pressure and process-candidate snapshots and returns a policy decision.
It does not own private LMKD structures and does not perform an independent
process-kill implementation.

### AOSP source adapter

Bridges AOSP LMKD lifecycle and decision events to the versioned AETHER Agent
ABI. It is compiled as part of the existing `lmkd` target. It is responsible
for converting source-native observations into normalized AETHER structures,
validating selected process identity, and delegating execution to LMKD's
existing reaper.

### Supervisor

Owns module lifecycle, configuration, health checks, diagnostics, and WebUI
snapshots. It monitors the in-process Runtime Agent and coordinates recovery
without becoming a second policy engine.

### Policy Core

Consumes normalized memory, pressure, and process observations. It is
independent of private LMKD structures and produces AETHER policy decisions.
Its input model must reflect the data used by AOSP LMKD, including memory
statistics, PSI or vmpressure signals, relevant VM statistics, process priority
metadata, and supported memory-event signals. Unsupported inputs remain
explicitly unavailable; they are not guessed.

### WebUI

A static manager-hosted module page. It reads snapshots published by the
Supervisor through the manager-provided bridge. It cannot write procfs, kernel
controls, or LMKD internals directly.

## Integration and hook strategy

1. **Primary: AOSP source integration.** Use the official AOSP lifecycle hooks
   where they fit, and add a dedicated policy-decision bridge at the AOSP
   source decision boundary. Build the Agent into the existing LMKD target.
2. **Secondary research only: runtime inline hooks.** ShadowHook remains the
   first candidate and Dobby the second for investigating prebuilt targets.
   These mechanisms are not the canonical universal path. A runtime hook must
   never be enabled on an unknown or unvalidated binary, and the two backends
   must never be installed against the same target function.
3. No target symbol, offset, private structure layout, or binary signature is
   assumed universal. AArch64 support does not imply binary compatibility
   across Android builds.

See `AOSP_INTEGRATION.md` and `HOOKING_EVALUATION.md`.

## Safety invariants

- Observe and validate before granting AETHER policy authority.
- Never allow AOSP LMKD policy and AETHER policy to act simultaneously.
- Fail closed to the original LMKD path on missing capabilities, invalid
  identity, protocol mismatch, integrity errors, Agent failure, or watchdog
  timeout.
- Keep process execution and accounting inside LMKD's established reaper path.
- Do not make blind writes to private process or kernel state.
- Keep a watchdog, kill switch, bounded event queue, and auditable transition
  log.
- Discover runtime capabilities; do not infer compatibility from OEM names.

## Current implementation: Serenity 0.9.2

The native module contains procfs observation, memory/PSI parsing, a pressure
classifier, a host-testable Fail Safe Gate state machine, bounded event
history, and a Supervisor that publishes snapshots for the manager-hosted
WebUI. The module remains an observation preview.

No in-process LMKD Runtime Agent, AOSP source adapter, policy-decision bridge,
hook installation, real Fail Safe Gate handoff, or device-level policy takeover
is implemented. The Supervisor remains in `LMKD_FALLBACK`.
