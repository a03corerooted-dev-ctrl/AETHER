# Serenity 0.9.1 — installer hotfix

This hotfix corrects the module installer permission sequence. The prior
customize.sh recursively assigned mode 0644 to every file, which removed the
execute bit from system/bin/aether-supervisor. The service then rejected the
binary with its -x check and exited before starting the local HTTP server.

The installer now restores executable mode 0755 to the supervisor after the
recursive permission pass, verifies that it is executable, and aborts if the
WebUI file is missing.

All Serenity 0.9 observation/supervisor limitations remain unchanged:
- Read-only procfs and PSI observation.
- Read-only loopback HTTP API and static WebUI.
- No in-process LMKD Runtime Agent.
- No active LMKD hooks; ShadowHook and Dobby remain evaluated candidates only.
- No takeover or policy handoff on-device.

This is an installer hotfix, not an LMKD integration release.
