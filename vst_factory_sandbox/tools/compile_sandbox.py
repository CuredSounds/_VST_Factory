#!/usr/bin/env python3
"""Agent-facing compile wrapper — uses CMake presets and emits compile_errors.json."""

import json
import os
import re
import subprocess
import sys

PRESET = os.environ.get("VST_FACTORY_PRESET", "dev")


def sandbox_dir() -> str:
    return os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def run_build() -> bool:
    root = sandbox_dir()
    errors_file = os.path.join(root, "compile_errors.json")

    if os.path.exists(errors_file):
        os.remove(errors_file)

    configure = subprocess.run(
        ["cmake", "--preset", PRESET],
        cwd=root,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if configure.returncode != 0:
        print("CMake configure failed!")
        parse_and_write_errors(configure.stderr, errors_file)
        return False

    build = subprocess.run(
        ["cmake", "--build", f"--preset", PRESET],
        cwd=root,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if build.returncode != 0:
        print("Compilation failed!")
        parse_and_write_errors(build.stdout + "\n" + build.stderr, errors_file)
        return False

    print("Compilation succeeded!")
    return True


def parse_and_write_errors(raw_output: str, target_json_path: str) -> None:
    pattern = re.compile(r"^(.+?):(\d+):(\d+):\s+(error|warning):\s+(.+)$")
    parsed_errors = []

    for line in raw_output.splitlines():
        match = pattern.match(line.strip())
        if match:
            filepath, line_num, col_num, error_type, message = match.groups()
            parsed_errors.append(
                {
                    "file": filepath,
                    "line": int(line_num),
                    "column": int(col_num),
                    "type": error_type,
                    "message": message,
                }
            )

    if not parsed_errors and raw_output.strip():
        parsed_errors.append(
            {
                "file": "CMake/Compiler",
                "line": 0,
                "column": 0,
                "type": "error",
                "message": "Raw compile output: " + raw_output.strip(),
            }
        )

    with open(target_json_path, "w", encoding="utf-8") as f:
        json.dump(parsed_errors, f, indent=2)
    print(f"Errors written to {target_json_path}")


if __name__ == "__main__":
    sys.exit(0 if run_build() else 1)
