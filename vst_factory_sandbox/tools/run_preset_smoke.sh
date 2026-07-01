#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SANDBOX_DIR="$(dirname "$SCRIPT_DIR")"

cd "$SANDBOX_DIR"

PYTHON="python3"
VENV_DIR="$SANDBOX_DIR/../.venv/bin"
if [[ -x "$VENV_DIR/python3.12" ]]; then
  PYTHON="$VENV_DIR/python3.12"
elif [[ -x "$VENV_DIR/python" ]]; then
  PYTHON="$VENV_DIR/python"
fi

if ! "$PYTHON" -c "import pytest" 2>/dev/null; then
  if [[ "${CI:-}" == "true" ]]; then
    echo "pytest required in CI. Run: pip install -r tools/requirements-test.txt"
    exit 1
  fi
  echo "pytest not installed — skipping preset smoke locally."
  exit 0
fi

if ! "$PYTHON" -c "import dawdreamer" 2>/dev/null; then
  if [[ "${CI:-}" == "true" ]]; then
    echo "dawdreamer required in CI but not installed."
    exit 1
  fi
  echo "dawdreamer not installed — skipping preset smoke locally."
  exit 0
fi

"$PYTHON" -m pytest Tests/PresetSmokeTests/ -v
echo "Preset smoke tests passed."
