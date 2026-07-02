#!/usr/bin/env python3
"""Scaffold a new VST Factory plugin repo from vst_factory_sandbox.

Example:
  python3 tools/scaffold_plugin.py --name "Tape Saturator" --out ../tape_saturator
  python3 tools/scaffold_plugin.py --name my_delay --company CuredSounds --out ../my_delay
"""

from __future__ import annotations

import argparse
import re
import shutil
import sys
from datetime import date
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
SANDBOX = REPO_ROOT / "vst_factory_sandbox"
AGENTS_TEMPLATE = REPO_ROOT / "_VST_Factory_Claude_docs" / "01_AGENTS_TEMPLATE.md"

SKIP_DIRS = {
    "build",
    "cmake-build-debug",
    ".venv",
    "__pycache__",
    ".git",
}
SKIP_FILES = {"compile_errors.json"}


def to_snake(name: str) -> str:
    text = re.sub(r"[^A-Za-z0-9]+", "_", name.strip())
    text = re.sub(r"_+", "_", text).strip("_").lower()
    if not text:
        raise ValueError("plugin name must contain at least one letter or digit")
    if text[0].isdigit():
        text = f"plugin_{text}"
    return text


def to_pascal(name: str) -> str:
    parts = re.split(r"[^A-Za-z0-9]+", name.strip())
    merged = "".join(p[:1].upper() + p[1:] for p in parts if p)
    if not merged:
        raise ValueError("plugin name must contain at least one letter or digit")
    if merged[0].isdigit():
        merged = "Plugin" + merged
    return merged


def to_display(name: str) -> str:
    if re.search(r"[\s_-]", name):
        parts = re.split(r"[\s_-]+", name.strip())
        return " ".join(p[:1].upper() + p[1:].lower() if p else "" for p in parts if p)
    # camelCase / PascalCase -> spaced words
    spaced = re.sub(r"([a-z0-9])([A-Z])", r"\1 \2", name.strip())
    return spaced[:1].upper() + spaced[1:]


def replacement_table(plugin_name: str, company: str) -> list[tuple[str, str]]:
    snake = to_snake(plugin_name)
    pascal = to_pascal(plugin_name)
    display = to_display(plugin_name)
    display_underscore = display.replace(" ", "_")

    return [
        ("VstFactorySandboxAudioProcessorEditor", f"{pascal}AudioProcessorEditor"),
        ("VstFactorySandboxAudioProcessor", f"{pascal}AudioProcessor"),
        ("VST_Factory_Sandbox", display_underscore),
        ("vst_factory_sandbox", snake),
        ("vst_factory_dsp", f"{snake}_dsp"),
        ("VSTFactory", pascal),
        ("VST Factory Sandbox", display),
        ("SonicDesign", company),
    ]


def should_skip(path: Path) -> bool:
    return any(part in SKIP_DIRS for part in path.parts) or path.name in SKIP_FILES


def copy_sandbox_tree(dest: Path) -> None:
    if dest.exists() and any(dest.iterdir()):
        raise SystemExit(f"refusing to scaffold into non-empty directory: {dest}")

    dest.mkdir(parents=True, exist_ok=True)

    for src in SANDBOX.rglob("*"):
        if should_skip(src.relative_to(SANDBOX)):
            continue
        rel = src.relative_to(SANDBOX)
        out = dest / rel
        if src.is_dir():
            out.mkdir(parents=True, exist_ok=True)
        else:
            out.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, out)


def apply_replacements(dest: Path, replacements: list[tuple[str, str]]) -> None:
    text_ext = {
        ".cpp", ".h", ".hpp", ".cmake", ".txt", ".md", ".yml", ".yaml",
        ".py", ".sh", ".json", ".xml",
    }
    for path in dest.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in text_ext:
            continue
        if should_skip(path.relative_to(dest)):
            continue
        original = path.read_text(encoding="utf-8")
        updated = original
        for old, new in replacements:
            updated = updated.replace(old, new)
        if updated != original:
            path.write_text(updated, encoding="utf-8")


def write_agents_md(dest: Path, plugin_name: str, company: str, plugin_type: str) -> None:
    display = to_display(plugin_name)
    pascal = to_pascal(plugin_name)
    template = AGENTS_TEMPLATE.read_text(encoding="utf-8")

    # Extract markdown body from fenced block in template doc.
    match = re.search(r"```markdown\n(.*?)```", template, re.DOTALL)
    body = match.group(1) if match else template

    body = body.replace("[Plugin Name]", display)
    body = body.replace(
        "[One paragraph: effect or synth, target sound/competitor, plugin formats shipped.]",
        (
            f"A {plugin_type} plugin scaffolded from _VST_Factory. "
            f"Replace this paragraph with the product brief (target sound, formats, competitors)."
        ),
    )
    body = body.replace("[e.g. `Source/Core/Parameters.h`]", "`Source/Core/Parameters.h`")
    body = body.replace("[path/to/test_or_ci]", "`tools/run_all_gates.sh`")
    body = body.replace("[e.g. `Softysn1::Params`]", f"`{pascal}::Params`")

    (dest / "AGENTS.md").write_text(body.strip() + "\n", encoding="utf-8")


def write_roadmap(dest: Path, plugin_name: str) -> None:
    display = to_display(plugin_name)
    today = date.today().isoformat()
    content = f"""# {display} Roadmap

No item moves to done without a linked verification artifact (test path or CI run).

## Phase 0 — Greenfield scaffold

- [x] Repo scaffolded from _VST_Factory sandbox ({today})
- [ ] First real DSP module + Catch2 tests
- [ ] CI workflow wired (`tools/run_all_gates.sh`)
- [ ] RT static scan clean (`tools/scan_rt_violations.py`)

Verification: `cmake --preset ci && cmake --build --preset ci && ./tools/run_all_gates.sh`
"""
    (dest / "ROADMAP.md").write_text(content, encoding="utf-8")


def write_changelog(dest: Path, plugin_name: str) -> None:
    display = to_display(plugin_name)
    today = date.today().isoformat()
    content = f"""# {display} Changelog

## [{today}] Init

- Scaffolded from `_VST_Factory/vst_factory_sandbox` via `tools/scaffold_plugin.py`.
"""
    (dest / "CHANGELOG.md").write_text(content, encoding="utf-8")


def write_readme(dest: Path, plugin_name: str) -> None:
    display = to_display(plugin_name)
    snake = to_snake(plugin_name)
    content = f"""# {display}

JUCE effect plugin built under the [_VST_Factory](https://github.com/CuredSounds/_VST_Factory) harness.

## Build

```bash
cmake --preset ci
cmake --build --preset ci
./tools/run_all_gates.sh
```

## Layout

```
Source/Core/     PluginProcessor, Parameters.h
Source/DSP/      Effect modules (start with GainProcessor stub)
Source/UI/       PluginEditor
Tests/DSP_Tests/ Catch2
```

See `AGENTS.md` and factory docs in `_VST_Factory/_VST_Factory_Claude_docs/`.
"""
    (dest / "README.md").write_text(content, encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--name", required=True, help="Plugin name (e.g. 'Tape Saturator')")
    parser.add_argument("--company", default="SonicDesign", help="JUCE COMPANY_NAME")
    parser.add_argument(
        "--type",
        choices=("effect", "synth"),
        default="effect",
        help="Plugin category for AGENTS.md blurb (default: effect)",
    )
    parser.add_argument(
        "--out",
        required=True,
        type=Path,
        help="Output directory for the new plugin repo",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    if not SANDBOX.is_dir():
        print(f"sandbox not found: {SANDBOX}", file=sys.stderr)
        return 1

    dest = args.out.expanduser().resolve()
    replacements = replacement_table(args.name, args.company)

    print(f"Scaffolding '{to_display(args.name)}' -> {dest}")
    copy_sandbox_tree(dest)
    apply_replacements(dest, replacements)
    write_agents_md(dest, args.name, args.company, args.type)
    write_roadmap(dest, args.name)
    write_changelog(dest, args.name)
    write_readme(dest, args.name)

    print("Done. Next steps:")
    print(f"  cd {dest}")
    print("  git init && git add .")
    print("  git config core.hooksPath .githooks")
    print("  cmake --preset ci && cmake --build --preset ci")
    print("  ./tools/run_all_gates.sh")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
