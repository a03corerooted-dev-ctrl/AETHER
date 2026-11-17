# Optional AOSP LMKD integration research

> **Architecture status:** This is an exploratory integration study, not a requirement for AETHER Core. The current project direction is an independent memory-management core with capability-detected platform adapters. No LMKD takeover, hook installation, or runtime injection is active. Any requirement below applies only if this optional integration path is deliberately pursued in a later milestone.

# AETHER AOSP LMKD integration

## Source baseline

Canonical upstream project:

- Repository: `platform/system/memory/lmkd`
- Audited AOSP main commit: `767ae2336a12f0c474b19dbd86d5bd5164dbf912`
- Source tree: https://android.googlesource.com/platform/system/memory/lmkd/+/refs/heads/main/
- LMKD implementation: https://android.googlesource.com/platform/system/memory/lmkd/+/refs/heads/main/lmkd.cpp
- Build definition: https://android.googlesource.com/platform/system/memory/lmkd/+/refs/heads/main/Android.bp
- Hook declarations: https://android.googlesource.com/platform/system/memory/lmkd/+/refs/heads/main/include/lmkd_hooks.h
- Init service definition: https://android.googlesource.com/platform/system/memory/lmkd/+/refs/heads/main/lmkd.rc

The AOSP main branch is the source reference, not a promise that every vendor
image contains an identical LMKD binary. AETHER must pin each source baseline
used for a build and keep source adapters versioned.

## Existing LMKD process

AOSP's init definition launches `/system/bin/lmkd` as a core service, with a
dedicated service identity, capabilities, task profile, and the `lmkd`
control socket. The source implementation owns the event loop, memory-pressure
monitor registration, process candidate data, kill accounting, and reaper.

The AETHER Runtime Agent belongs inside this existing process. It must not
launch a second LMKD, replace the init service with an unrelated daemon, or
implement a competing process-kill executor.

## Existing AOSP hook surface

The upstream `include/lmkd_hooks.h` currently declares:

- `lmkd_update_props_hook()`
- `lmkd_init_hook()`
- `lmkd_free_memory_before_kill_hook(...)`
- `lmkd_no_kill_candidates_hook()`

AOSP's `Android.bp` defines the `lmkd_hooks_cc_defaults` Soong module type.
When its `use_hooks` configuration is enabled, the build defines
`LMKD_USE_HOOKS` and links `liblmkdhooks`. Otherwise, the header provides
no-op inline implementations.

Important limitation: this is a build-time extension. It is not an Android
runtime plugin loader. The pre-kill callback runs after LMKD has already
selected a process; its positive result means memory was freed elsewhere and
that particular kill should be skipped. The no-candidates callback is also a
notification, not a replacement for candidate selection. Neither callback
gives AETHER sole ownership of LMKD's policy.

## Possible future source-level adapter

AETHER must add a dedicated, versioned decision bridge to the AOSP LMKD source.
The bridge is invoked from the memory-pressure decision path before the normal
LMKD candidate-selection algorithm. It is a semantic source integration point,
not a binary offset or exported private function.

The Agent ABI must use bounded C-compatible value structures. It must not
expose pointers to LMKD-private objects or depend on the layout of AOSP's
internal `struct proc`. The source adapter performs the translation.

At each eligible decision cycle:

1. LMKD gathers the authoritative current pressure state and a bounded set of
   eligible process candidates from its existing internal data.
2. The adapter validates the Agent's presence, protocol version, integrity,
   and watchdog state.
3. In `LMKD_FALLBACK`, the original AOSP decision path runs unchanged.
4. In `AETHER_ACTIVE`, AETHER returns either no action or a selected candidate
   from the supplied candidate set. LMKD's original policy-selection algorithm
   does not run for that cycle.
5. Before execution, LMKD revalidates process identity/lifetime and delegates
   the operation to its own reaper. Existing pidfd handling, accounting, and
   event reporting remain in LMKD.
6. Any timeout, malformed response, stale candidate, Agent fault, or
   integrity failure closes the AETHER authority window and returns the next
   decision cycle to the original AOSP path.

The handoff must be serialized on the LMKD decision thread or through an
explicitly synchronized contract. The AOSP policy path and AETHER must never
issue competing decisions for one pressure event.

## Source-derived inputs

The adapter and policy model should follow the actual AOSP LMKD data sources
and capabilities instead of inventing a parallel device-specific discovery
scheme. Relevant upstream inputs include:

- Memory and VM data read from procfs, including meminfo, zoneinfo, and vmstat.
- PSI or vmpressure signals, selected according to available kernel support.
- Memory-event facilities when exposed by the platform/kernel.
- Process priority and OOM adjustment metadata maintained by LMKD.
- Process identity/lifetime mechanisms such as pidfd when supported.
- LMKD's own reaper and accounting path for execution.

Capability absence is represented explicitly. AETHER must not silently treat
missing PSI, pidfd, or memory-event support as if the capability existed.

## Universal compatibility boundary

The universal path means source-level integration against the AOSP LMKD
contract: no OEM-name tables, per-device binary offsets, or hard-coded symbol
addresses. The build adapter compiles against the source revision it targets.

A prebuilt LMKD binary cannot retroactively acquire a compile-time hook by
installing a separate library. A root module targeting arbitrary prebuilt
images therefore needs a distinct runtime integration mechanism. Runtime
inline hooks may be researched separately, but they are not source-level AOSP
APIs and cannot be advertised as universally compatible without validating the
actual target binary and its security/runtime constraints.

## Current implementation status

Serenity 0.9.2 does not yet include this source adapter, the Runtime Agent ABI,
or the LMKD decision bridge. The module is still observation-only and keeps
`LMKD_FALLBACK`. This document records the implementation direction and the
upstream source findings; it does not claim that integration is already built
or tested.
