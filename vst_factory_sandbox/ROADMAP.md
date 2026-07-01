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
- [ ] Preset smoke green in CI (DawDreamer + baseline manifest)
- [ ] pluginval green in CI

## Phase 1 — Template hardening

- [ ] CMakePresets `dev` documented in README
- [ ] CLAP format (optional, pinned commit)
- [ ] Plugin-level processBlock allocation test (JUCE integration)
