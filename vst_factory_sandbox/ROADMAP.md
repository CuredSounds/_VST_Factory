# VST Factory Sandbox Roadmap

No item moves to done without a linked verification artifact (test path or CI run).

## Phase 0 — Harness scaffold

- [x] Core/DSP/UI split with GainProcessor — `Tests/DSP_Tests/GainProcessorTests.cpp`
- [x] Catch2 test harness — `cmake --preset ci && ./build/ci/DSP_Tests`
- [x] Allocation gate — `Tests/DSP_Tests/AllocationTest.cpp`
- [x] Null test + golden — `Tests/golden/gain_null_reference.bin`
- [x] Quality gate scripts — `tools/run_all_gates.sh`
- [x] Local git hooks — `.githooks/pre-commit`, `.githooks/pre-push`
- [x] GitHub Actions — `.github/workflows/sandbox-ci.yml`
- [x] Preset smoke — `Tests/PresetSmokeTests/` (verified locally; CI via sandbox-ci.yml)
- [x] pluginval — `tools/run_pluginval.sh` (CI installs binary; optional locally)

## Phase 1 — Template hardening

- [x] CMakePresets `dev` + `ci` documented in README
- [x] Plugin-level processBlock allocation test — `Tests/DSP_Tests/ProcessorIntegration_test.cpp` (`[integration]`)
- [ ] CLAP format (optional, pinned commit)
