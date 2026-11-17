#!/system/bin/sh
MODDIR="$(cd "$(dirname "$0")" && pwd)"
BIN="$MODDIR/system/bin/aether-supervisor"
WEB="$MODDIR/web"
LOG="$MODDIR/aether.log"
PIDFILE="$MODDIR/supervisor.pid"
if [ "$(getprop ro.product.cpu.abi)" != "arm64-v8a" ]; then
  echo "AETHER: unsupported ABI; only arm64-v8a is accepted" >> "$LOG"
  exit 0
fi
if [ ! -x "$BIN" ]; then
  echo "AETHER: supervisor binary missing; install an ARM64 package built for this device" >> "$LOG"
  exit 0
fi
while [ "$(getprop sys.boot_completed)" != "1" ]; do sleep 2; done
"$BIN" --web-root "$WEB" --port 8765 >> "$LOG" 2>&1 &
echo $! > "$PIDFILE"
