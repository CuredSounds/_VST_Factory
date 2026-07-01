#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SANDBOX_DIR="$(dirname "$SCRIPT_DIR")"
TEST_BIN="$SANDBOX_DIR/build/ci/DSP_Tests"

cd "$SANDBOX_DIR"

if [[ ! -x "$TEST_BIN" ]]; then
  echo "DSP_Tests binary not found. Run tools/run_build.sh first."
  exit 1
fi

if [[ "${UPDATE_GOLDENS:-}" == "1" ]]; then
  export UPDATE_GOLDENS=1
  "$TEST_BIN" "[dsp][null]"
  echo "Golden references updated."
  exit 0
fi

"$TEST_BIN" "[dsp][null]"
echo "Null tests passed."
