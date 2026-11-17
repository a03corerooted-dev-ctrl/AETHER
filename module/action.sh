#!/system/bin/sh
MODDIR="${0%/*}"
BIN="$MODDIR/system/bin/aether-supervisor"
if [ ! -x "$BIN" ]; then
  echo "AETHER Supervisor binary is missing or not executable."
  exit 1
fi
echo "AETHER Serenity 0.9.2 — one-shot diagnostic snapshot"
"$BIN" --once
