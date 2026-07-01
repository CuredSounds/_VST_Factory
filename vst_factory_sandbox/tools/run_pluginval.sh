#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SANDBOX_DIR="$(dirname "$SCRIPT_DIR")"
PLUGIN_PATH="$SANDBOX_DIR/build/ci/vst_factory_sandbox_artefacts/Release/VST3/VST_Factory_Sandbox.vst3"
LOG_DIR="$SANDBOX_DIR/build/ci/pluginval-logs"
STRICTNESS="${PLUGINVAL_STRICTNESS:-8}"

cd "$SANDBOX_DIR"

if [[ ! -d "$PLUGIN_PATH" ]]; then
  echo "VST3 not found at $PLUGIN_PATH — run tools/run_build.sh first."
  exit 1
fi

if ! command -v pluginval >/dev/null 2>&1; then
  if [[ "${CI:-}" == "true" ]]; then
    echo "pluginval required in CI but not found."
    exit 1
  fi
  echo "pluginval not installed. Install from: https://github.com/Tracktion/pluginval/releases"
  echo "Skipping pluginval locally."
  exit 0
fi

mkdir -p "$LOG_DIR"

pluginval \
  --validate "$PLUGIN_PATH" \
  --strictness-level "$STRICTNESS" \
  --skip-gui-tests \
  --timeout-ms 30000 \
  --output-dir "$LOG_DIR"

echo "pluginval passed (strictness $STRICTNESS). Logs: $LOG_DIR"
