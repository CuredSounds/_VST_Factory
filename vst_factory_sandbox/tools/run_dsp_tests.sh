#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SANDBOX_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$SANDBOX_DIR/build/ci"
TEST_BIN="$BUILD_DIR/DSP_Tests"
RESULTS="$BUILD_DIR/test-results.xml"

cd "$SANDBOX_DIR"

if [[ ! -x "$TEST_BIN" ]]; then
  echo "DSP_Tests binary not found. Run tools/run_build.sh first."
  exit 1
fi

"$TEST_BIN" --reporter junit --out "$RESULTS"
echo "DSP tests passed. Results: $RESULTS"
