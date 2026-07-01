#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SANDBOX_DIR="$(dirname "$SCRIPT_DIR")"

cd "$SANDBOX_DIR"

if ! command -v ninja >/dev/null 2>&1; then
  echo "ninja not found; install via: brew install ninja"
  exit 1
fi

cmake --preset ci
cmake --build --preset ci

echo "Build succeeded (ci preset)."
