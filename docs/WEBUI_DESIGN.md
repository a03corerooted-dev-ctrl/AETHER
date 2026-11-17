# AETHER WebUI design

## Hosting contract

AETHER's UI is a static module WebUI. Its entry point is webroot/index.html at
the module root. A compatible manager loads that directory inside its own
WebView. AETHER MUST NOT create an HTTP server, TCP/Unix listener, localhost
endpoint, embedded browser, or external host process.

KernelSU and KernelSU Next expose the ksu JavaScript bridge to module WebUI
pages. AETHER uses the documented ksu.exec(command, options, callback) form
to read fixed-path JSON snapshots. No page-controlled command fragments are
interpolated into the shell command.

Magisk's documented module interaction is action.sh, displayed as an Action
button in the Magisk app. Magisk does not provide the same integrated module
WebView host/API. The AETHER Magisk action therefore prints one-shot
diagnostics in the manager console; it is not presented as a WebUI.

## Data path

1. The Supervisor samples procfs and evaluates the read-only pressure classifier.
2. It atomically publishes status.json, memory.json, processes.json, and
   events.json under /data/adb/modules/aether/state/.
3. The manager WebView calls ksu.exec to read those fixed files.
4. The UI parses and renders the snapshots; it does not connect to a network endpoint.

Snapshot files are replaced by rename after a complete write so the WebView
does not observe partially written JSON. The state directory is root-owned and
mode 0750; snapshot files are mode 0640. Manager bridge execution is privileged.

## Views

- Overview: authority, Fail Safe Gate status, memory and pressure.
- Memory: total, available, free, cached, swap, PSI and validity.
- Processes: PID, UID, name, RSS, oom_score_adj, and state.
- Event history: bounded diagnostic events.

## Security boundary

The WebUI is currently read-only. It must never write procfs, kernel controls,
LMKD internals, or policy configuration. Future mutating controls require
explicit authorization, strict input validation, rate limits, and mandatory
Fail Safe Gate mediation. No localhost HTTP service is permitted as a fallback.
