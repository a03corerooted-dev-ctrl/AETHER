# AETHER WebUI design

The WebUI is a Supervisor client, not a privileged control surface. Serenity 0.9
serves it from the Supervisor over a loopback-only, read-only HTTP API bound to
127.0.0.1. The API is not authenticated yet; local applications on the same
device may be able to access it. It must never bind to a wildcard or LAN address.
Do not add mutating endpoints until authentication and Fail Safe Gate mediation
are implemented.

## Primary views
- **Overview:** authority (`AETHER_ACTIVE`, `LMKD_FALLBACK`, `TRANSITION`),
  Runtime Agent health, Fail Safe Gate status, uptime, and last transition reason.
- **Memory:** total, available, free, cached, swap, and memory PSI (`some`/`full`)
  with timestamps and source-validity indicators.
- **Processes:** PID, UID, name, RSS, `oom_score_adj`, and state; unavailable
  fields are shown as unavailable, never fabricated.
- **Policy timeline:** timestamped AETHER decisions, inputs, outcomes, and authority
  state at the time.
- **Diagnostics:** hook backend identity/version, capability checks, watchdog
  events, rejected targets, and bounded diagnostic export.
- **Configuration:** observation mode, sampling interval, logging level, activation
  status. Policy controls stay disabled until Fail Safe Gate validation.

## Implemented endpoints in Serenity 0.9
- `GET /` — dashboard HTML.
- `GET /api/v1/status`
- `GET /api/v1/memory`
- `GET /api/v1/processes`
- `GET /api/v1/events`

All API requests are GET-only. Other methods return 405. Diagnostics and
configuration endpoints are future work; no configuration mutation is exposed.

## Visual hierarchy
Authority state is the most prominent status element. Memory/pressure metrics are
second. Process details and event history are subordinate. Show clear units,
data age, and distinguish measured values from unavailable/inferred values.

## Future security requirements
Before the WebUI exposes policy-affecting controls, the Supervisor must add
per-install authentication, request validation, rate limits, and mandatory
Fail Safe Gate mediation. The WebUI must never write procfs, kernel controls,
or LMKD internals directly.
