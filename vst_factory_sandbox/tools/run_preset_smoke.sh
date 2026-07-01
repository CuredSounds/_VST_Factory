#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SANDBOX_DIR="$(dirname "$SCRIPT_DIR")"

cd "$SANDBOX_DIR"

if ! python3 -c "import pytest" 2>/dev/null; then
  if [[ "${CI:-}" == "true" ]]; then
    echo "pytest required in CI. Run: pip install -r tools/requirements-test.txt"
    exit 1
  fi
  echo "pytest not installed — skipping preset smoke locally."
  exit 0
fi

if ! python3 -c "import dawdreamer" 2>/dev/null; then
  if [[ "${CI:-}" == "true" ]]; then
    echo "dawdreamer required in CI but not installed."
    exit 1
  fi
  echo "dawdreamer not installed — skipping preset smoke locally."
  exit 0
fi

python3 -m pytest Tests/PresetSmokeTests/ -v
echo "Preset smoke tests passed."
