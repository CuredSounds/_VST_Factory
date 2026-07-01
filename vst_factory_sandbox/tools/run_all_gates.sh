#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== 1/6 Build ==="
"$SCRIPT_DIR/run_build.sh"

echo "=== 2/6 Catch2 DSP tests ==="
"$SCRIPT_DIR/run_dsp_tests.sh"

echo "=== 3/6 Preset smoke tests ==="
"$SCRIPT_DIR/run_preset_smoke.sh"

echo "=== 4/6 Allocation gate ==="
"$SCRIPT_DIR/run_allocation_gate.sh"

echo "=== 5/6 Null tests ==="
"$SCRIPT_DIR/run_null_tests.sh"

echo "=== 6/6 pluginval ==="
"$SCRIPT_DIR/run_pluginval.sh"

echo "All quality gates passed."
