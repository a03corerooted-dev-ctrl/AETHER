#!/system/bin/sh
MODDIR="$(cd "$(dirname "$0")" && pwd)"
BIN="$MODDIR/system/bin/aether-supervisor"
LOG="$MODDIR/aether.log"
PIDFILE="$MODDIR/supervisor.pid"
if [ "$(getprop ro.product.cpu.abi)" != "arm64-v8a" ]; then
  echo "AETHER: unsupported ABI; only arm64-v8a is accepted" >> "$LOG"
  exit 0
fi
if [ ! -x "$BIN" ]; then
  echo "AETHER: supervisor binary missing or not executable" >> "$LOG"
  exit 0
fi
while [ "$(getprop sys.boot_completed)" != "1" ]; do sleep 2; done
"$BIN" --state-dir "$MODDIR/state" >> "$LOG" 2>&1 &
echo $! > "$PIDFILE"
