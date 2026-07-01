# VST Factory Sandbox

Reference gain/bypass effect plugin — the harness proving ground for _VST_Factory.

## Build

```bash
cmake --preset ci
cmake --build --preset ci
```

Or: `./tools/run_build.sh`

## Tests

```bash
./tools/run_dsp_tests.sh
./tools/run_allocation_gate.sh
./tools/run_null_tests.sh
./tools/run_all_gates.sh   # full stack
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
