# Android and Linux platform limits

This document records constraints that shape AETHER's OEM-independent design. Capability availability is a runtime property; Android version or ARM64 alone is not sufficient proof that an interface exists.

## Android process and memory ownership

### Android 11+: Cached App Freezer

Android 11 introduced a cached-app freezer backed by a kernel cgroup freezer. Android 14+ additionally performs lifecycle coordination around freezing, compaction, and swap. AETHER must observe or integrate through platform-owned interfaces rather than independently moving application processes into frozen cgroups.

Reference: https://source.android.com/docs/core/perf/cached-apps-freezer

### Android 17+: Memory Limiter

Android 17 includes a Memory Limiter integrated with ActivityManagerService and cgroup v2. Its controls include memory.high, memory.max, and memory.swap.max. App budgets belong to the app. Device implementations must not bypass platform memory limits or set, raise, clear, or ignore app-owned budgets on the app's behalf.

AETHER must treat those budgets as platform policy. It may observe supported accounting data, but Core must not rewrite those limits or install allowlists that bypass them.

References:
- https://source.android.com/docs/core/perf/memory-limiter
- https://source.android.com/docs/compatibility/17/android-17-cdd

### Android 17+: system memory budgets

AOSP documents SystemMemoryBudgetManager protection floors using cgroup v2 memory.min and system-app ceilings using memory.high. The hierarchy and effective protection are non-trivial; writing a leaf value without understanding parent limits may not produce the intended protection.

AOSP also documents a targeted file-cache reclaim using memory.reclaim with swappiness=0 on Linux 6.2+ and explicitly skips the operation on older kernels that reject that option. This is why AETHER requires a verified file-only reclaim capability and an adapter-selected cgroup instead of issuing a generic reclaim write.

References:
- https://source.android.com/docs/core/perf/system-memory-budgets
- https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html

## Kernel telemetry and reclaim

### PSI

Memory PSI is exported through /proc/pressure/memory when kernel support is enabled. some and full measure different stall conditions; they are not interchangeable with a free-memory percentage. PSI is optional and must be feature-detected.

Reference: https://www.kernel.org/doc/html/latest/accounting/psi.html

### cgroup v2

cgroup v2 exposes hierarchical memory accounting and controls such as memory.current, memory.stat, memory.events, and memory.reclaim, but the controller and files must be present and accessible. Android device layouts and permissions vary. AETHER must discover the actual hierarchy and must not hard-code a path such as /sys/fs/cgroup/apps/uid_.../pid_... as universal.

Reference: https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html

### ZRAM

ZRAM is a compressed RAM-backed block device, commonly used as swap on Android. Its presence, device index, algorithm, and exported statistics depend on the kernel and device configuration. AETHER Core treats ZRAM as an optional telemetry/backend capability and will not resize, reset, or reconfigure it in the current milestone.

Reference: https://www.kernel.org/doc/html/latest/admin-guide/blockdev/zram.html

### DAMON

DAMON_RECLAIM is an optional kernel facility for proactive reclaim of cold regions. It requires the relevant kernel configuration and its own runtime parameters/quotas. It is not a universal Android facility and is not part of Core v1.

Reference: https://www.kernel.org/doc/html/latest/admin-guide/mm/damon/reclaim.html

## Current AETHER capability matrix

| Facility | What AETHER may do now | Future condition |
|---|---|---|
| /proc/meminfo | Read and normalize available fields | Field presence varies |
| memory PSI | Read and classify when present | Requires kernel PSI support |
| /proc/<pid>/status | Bounded read of RSS and related values | Access and fields vary |
| cgroup v2 accounting | Not yet collected by live adapter | Detect controller and readable files |
| memory.reclaim | Plan only; no write | Verified file-only mode, safe target, permissions |
| ZRAM | No writes; telemetry adapter not yet implemented | Discover actual device and stats |
| DAMON_RECLAIM | No control | Kernel config and interface validation |
| app lifecycle/freeze | No direct control | Platform-owned source integration only |

A capability should be enabled only after its interface has been read and validated on the running device. Kernel version strings, OEM identity, and assumed Android API levels are not sufficient by themselves.
