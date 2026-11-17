# AETHER WebUI design

The WebUI is a Supervisor client, not a privileged control surface. It uses a
local authenticated API and never writes kernel controls or LMKD memory directly.

## Primary views
- **Overview:** authority (\`AETHER_ACTIVE\`, \`LMKD_FALLBACK\`, \`TRANSITION\`),
  Runtime Agent health, Fail Safe Gate status, uptime, and last transition reason.
- **Memory:** total, available, free, cached, swap, and memory PSI (\`some\`/\`full\`)
  with timestamps and source-validity indicators.
- **Processes:** PID, UID, name, RSS, \`oom_score_adj\`, and state; unavailable
  fields are shown as unavailable, never fabricated.
- **Policy timeline:** timestamped AETHER decisions, inputs, outcomes, and authority
  state at the time.
- **Diagnostics:** hook backend identity/version, capability checks, watchdog
  events, rejected targets, and bounded diagnostic export.
- **Configuration:** observation mode, sampling interval, logging level, activation
  status. Policy controls stay disabled until Fail Safe Gate validation.

## Visual hierarchy
Authority state is the most prominent status element. Memory/pressure metrics are
second. Process details and event history are subordinate. Show clear units,
data age, and distinguish measured values from unavailable/inferred values.

## Planned API
- \`GET /api/v1/status\`
- \`GET /api/v1/memory\`
- \`GET /api/v1/processes\`
- \`GET /api/v1/events\`
- \`GET /api/v1/diagnostics\`
- \`GET /api/v1/config\`
- \`PUT /api/v1/config\` (Supervisor validation; activation withheld until safe)

The API is local-only, versioned, bounded, and read-only by default. Any later
control endpoint must be mediated by the Supervisor and Fail Safe Gate.
