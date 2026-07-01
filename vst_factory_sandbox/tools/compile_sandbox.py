#!/usr/bin/env python3
"""Agent-facing compile wrapper — delegates to run_build.sh and emits compile_errors.json."""

import json
import os
import re
import subprocess
import sys

PRESET = os.environ.get("VST_FACTORY_PRESET", "ci")


def sandbox_dir() -> str:
    return os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def run_build() -> bool:
    root = sandbox_dir()
    errors_file = os.path.join(root, "compile_errors.json")
    run_build_sh = os.path.join(root, "tools", "run_build.sh")

    if os.path.exists(errors_file):
        os.remove(errors_file)

    env = os.environ.copy()
    env["VST_FACTORY_PRESET"] = PRESET

    if os.path.isfile(run_build_sh):
        result = subprocess.run(
            [run_build_sh],
            cwd=root,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            env=env,
        )
        if result.returncode == 0:
            print(result.stdout)
            print("Compilation succeeded!")
            return True
        print(result.stdout)
        parse_and_write_errors(result.stdout, errors_file)
        return False

    # Fallback if script missing
    for cmd in (
        ["cmake", "--preset", PRESET],
        ["cmake", "--build", "--preset", PRESET],
    ):
        result = subprocess.run(
            cmd, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True
        )
        if result.returncode != 0:
            parse_and_write_errors(result.stdout, errors_file)
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
                "message": "Raw compile output: " + raw_output.strip()[-4000:],
            }
        )

    with open(target_json_path, "w", encoding="utf-8") as f:
        json.dump(parsed_errors, f, indent=2)
    print(f"Errors written to {target_json_path}")


if __name__ == "__main__":
    sys.exit(0 if run_build() else 1)
