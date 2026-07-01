#!/usr/bin/env python3
import os
import sys
import subprocess
import re
import json

def compile_sandbox():
    sandbox_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    build_dir = os.path.join(sandbox_dir, "build")
    errors_file = os.path.join(sandbox_dir, "compile_errors.json")

    # Clear previous error log
    if os.path.exists(errors_file):
        os.remove(errors_file)

    # 1. Run CMake configuration if needed
    if not os.path.exists(build_dir):
        print("Configuring CMake...")
        res = subprocess.run(
            ["cmake", "-B", "build", "-S", "."],
            cwd=sandbox_dir,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        if res.returncode != 0:
            print("CMake configuration failed!")
            parse_and_write_errors(res.stderr, errors_file)
            return False

    # 2. Run Compilation
    print("Compiling project...")
    res = subprocess.run(
        ["cmake", "--build", "build"],
        cwd=sandbox_dir,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    if res.returncode != 0:
        print("Compilation failed!")
        parse_and_write_errors(res.stdout + "\n" + res.stderr, errors_file)
        return False

    print("Compilation succeeded!")
    return True

def parse_and_write_errors(raw_output, target_json_path):
    # Match pattern for GCC/Clang errors: filepath:line:col: [error|warning]: message
    pattern = re.compile(r"^(.+?):(\d+):(\d+):\s+(error|warning):\s+(.+)$")
    parsed_errors = []

    for line in raw_output.splitlines():
        match = pattern.match(line.strip())
        if match:
            filepath, line_num, col_num, error_type, message = match.groups()
            parsed_errors.append({
                "file": filepath,
                "line": int(line_num),
                "column": int(col_num),
                "type": error_type,
                "message": message
            })

    # If the parser couldn't structuralize it but there was a failure, add raw context
    if not parsed_errors and raw_output.strip():
        parsed_errors.append({
            "file": "CMake/Compiler",
            "line": 0,
            "column": 0,
            "type": "error",
            "message": "Raw compile output: " + raw_output.strip()
        })

    with open(target_json_path, "w") as f:
        json.dump(parsed_errors, f, indent=2)
    print(f"Errors written to {target_json_path}")

if __name__ == "__main__":
    success = compile_sandbox()
    sys.exit(0 if success else 1)
