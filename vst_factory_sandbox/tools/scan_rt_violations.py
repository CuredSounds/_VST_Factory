#!/usr/bin/env python3
"""Scan Source/ for common real-time safety anti-patterns."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Source"

PATTERNS: list[tuple[str, re.Pattern[str]]] = [
    (
        "juce::String in processBlock (heap allocation risk)",
        re.compile(r"void\s+\w+::processBlock\b[^{]*\{[^}]*juce::String", re.DOTALL),
    ),
    (
        "std::async (blocking/async on audio thread risk)",
        re.compile(r"std::async"),
    ),
    (
        "Raw string literal APVTS lookup (use Parameters.h constants)",
        re.compile(r'getRawParameterValue\s*\(\s*"'),
    ),
]

SKIP_FILES = {"Parameters.h"}


def scan_file(path: Path) -> list[str]:
    text = path.read_text(encoding="utf-8", errors="replace")
    issues: list[str] = []
    for label, pattern in PATTERNS:
        if pattern.search(text):
            issues.append(label)
    return issues


def main() -> int:
    if not SOURCE.exists():
        print(f"No Source/ directory at {SOURCE}")
        return 0

    failures: list[str] = []
    for path in sorted(SOURCE.rglob("*")):
        if path.suffix not in {".cpp", ".h", ".hpp", ".mm"}:
            continue
        if path.name in SKIP_FILES:
            continue
        for issue in scan_file(path):
            failures.append(f"{path.relative_to(ROOT)}: {issue}")

    if failures:
        print("RT guardrail scan FAILED:")
        for line in failures:
            print(f"  - {line}")
        return 1

    print("RT guardrail scan passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
