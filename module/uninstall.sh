#!/system/bin/sh
MODDIR="$(cd "$(dirname "$0")" && pwd)"
PIDFILE="$MODDIR/supervisor.pid"
if [ -f "$PIDFILE" ]; then
  PID="$(cat "$PIDFILE" 2>/dev/null)"
  case "$PID" in ''|*[!0-9]*) ;; *) kill "$PID" 2>/dev/null ;; esac
  rm -f "$PIDFILE"
fi
rm -rf "$MODDIR/state"
