# VST Factory Sandbox

Reference gain/bypass effect plugin — the harness proving ground for _VST_Factory.

## Build

```bash
cmake --preset ci          # Release + tests (CI)
cmake --build --preset ci

cmake --preset dev         # Debug (local iteration)
cmake --build --preset dev
```

Or: `./tools/run_build.sh` (uses `ci` preset; override with `VST_FACTORY_PRESET=dev`)

## Tests

```bash
./tools/run_dsp_tests.sh
./tools/run_allocation_gate.sh
./tools/run_null_tests.sh
./tools/run_all_gates.sh   # full stack
```

Preset smoke tests need **Python 3.12** (dawdreamer has no 3.13 wheels):

```bash
python3.12 -m venv .venv
source .venv/bin/activate
pip install -r tools/requirements-test.txt
./tools/run_preset_smoke.sh
```

## Agent compile feedback

```bash
python3 tools/compile_sandbox.py
# reads compile_errors.json on failure
```

## Layout

```
Source/Core/     PluginProcessor, Parameters.h
Source/DSP/      GainProcessor (pure C++)
Source/UI/       PluginEditor
Tests/DSP_Tests/ Catch2
```

See `AGENTS.md` and `../_VST_Factory_Claude_docs/` for full factory spec.
