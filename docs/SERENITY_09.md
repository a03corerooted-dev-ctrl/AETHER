# Serenity 0.9.2 — manager-hosted WebUI migration

Serenity 0.9.2 removes the embedded HTTP server and all loopback networking.
The WebUI now follows the KernelSU module layout: static assets live under
webroot/ with index.html as the entry point. KernelSU and KernelSU Next host
that page in their Manager WebView and expose the ksu JavaScript bridge.

The Supervisor publishes four complete JSON snapshots using temporary files
and atomic rename. The WebUI reads those fixed files through the manager bridge.
Magisk receives action.sh for one-shot diagnostics because its documented
module interface is an Action button/console, not an integrated module WebUI.

This migration changes only UI hosting and telemetry transport. It does not
implement the LMKD Runtime Agent, hooks, or policy-authority handoff.
