# AETHER architecture

## Design objective

AETHER is designed as a userspace policy engine for Android memory management.
Its intended integration point is the already-running LMKD process. The project
does not intend to ship a replacement daemon as its primary architecture.

The design separates policy from privileged integration. AETHER computes and
validates decisions; a narrowly scoped runtime adapter is responsible for
communicating with the target process and must reject unsupported states.

## Logical components

### 1. Supervisor

The external supervisor owns module lifecycle, configuration, health checks,
diagnostics, and the future WebUI service. It starts and monitors the native
runtime component and provides a recovery path.

### 2. Runtime agent

The in-process agent is the planned integration boundary with the existing LMKD
process. It will expose a versioned, minimal interface to the policy engine.
No injection or hook implementation is included in Foundation 0.1.

### 3. Policy core

The native policy core consumes normalized process and memory-pressure observations.
It produces policy decisions without depending directly on private LMKD structures.
This separation allows the decision logic to be unit-tested independently.

### 4. WebUI

The WebUI is a future management surface. It must communicate with the supervisor
through a documented local API; it must not write kernel or LMKD state directly.

## Runtime safety invariants

- **Observe before intervene:** initial runtime integration must support observation-only mode.
- **Fail closed:** missing symbols, unknown layouts, invalid protocol versions, or failed
  integrity checks disable intervention.
- **No blind writes:** runtime changes require validated targets and explicit ownership.
- **Recoverability:** the supervisor needs a kill switch, watchdog, and a way to return
  the agent to a non-intervening state.
- **Bounded impact:** policy decisions must be rate-limited and auditable.
- **No OEM assumptions:** capabilities are discovered at runtime, not inferred from
  manufacturer names or marketing version strings.

## Compatibility boundary

AArch64 is the only Android ABI targeted by this project. That does not make private
LMKD implementation details stable. Binary layout, symbols, compiler options, SELinux
policy, and daemon behavior can differ between Android releases and vendor builds.
A single backend is the design goal, but activation must be based on runtime
verification rather than a device profile or an assumed universal offset.

## Current implementation

Foundation 0.1 provides a small C++ lifecycle core and host-side tests. It does not
yet collect Android memory metrics, enumerate processes, communicate with LMKD,
inject code, or enforce memory policies.
