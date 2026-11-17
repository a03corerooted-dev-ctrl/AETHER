#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cmake -S "$ROOT" -B "$ROOT/build-host" -DAETHER_BUILD_TESTS=ON -DAETHER_STRICT_WARNINGS=ON
cmake --build "$ROOT/build-host" --parallel
ctest --test-dir "$ROOT/build-host" --output-on-failure
"$ROOT/build-host/native/aether-supervisor" --version
