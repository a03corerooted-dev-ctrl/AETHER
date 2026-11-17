#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NDK="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}"
if [[ -z "$NDK" || ! -f "$NDK/build/cmake/android.toolchain.cmake" ]]; then
  echo "Set ANDROID_NDK_HOME to a valid Android NDK directory." >&2
  exit 2
fi
BUILD="$ROOT/build-android"
STAGE="$ROOT/build/module-stage"
OUT="$ROOT/dist/AETHER-Serenity-0.9.2-arm64.zip"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-23 -DAETHER_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" --parallel
rm -rf "$STAGE" "$ROOT/dist"
mkdir -p "$STAGE/system/bin" "$STAGE/webroot" "$ROOT/dist"
cp "$ROOT/module/module.prop" "$STAGE/module.prop"
cp "$ROOT/module/service.sh" "$STAGE/service.sh"
cp "$ROOT/module/uninstall.sh" "$STAGE/uninstall.sh"
cp "$ROOT/module/customize.sh" "$STAGE/customize.sh"
cp "$ROOT/module/action.sh" "$STAGE/action.sh"
cp "$BUILD/native/aether-supervisor" "$STAGE/system/bin/aether-supervisor"
cp "$ROOT/module/webroot/index.html" "$STAGE/webroot/index.html"
chmod 0755 "$STAGE" "$STAGE/system" "$STAGE/system/bin" "$STAGE/webroot"
chmod 0755 "$STAGE/service.sh" "$STAGE/uninstall.sh" "$STAGE/customize.sh" "$STAGE/action.sh" "$STAGE/system/bin/aether-supervisor"
chmod 0644 "$STAGE/module.prop" "$STAGE/webroot/index.html"
if grep -R -nE '127\.0\.0\.1|localhost|--port|HttpServer|http_server' "$STAGE"; then
  echo "Unexpected HTTP-host dependency found in staged module." >&2
  exit 3
fi
(cd "$STAGE" && zip -qr "$OUT" .)
unzip -Z1 "$OUT" | grep -qx 'webroot/index.html'
echo "Package: $OUT"
