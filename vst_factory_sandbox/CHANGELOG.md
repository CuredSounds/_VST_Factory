# Changelog — VST Factory Sandbox

## [2026-07-01] Harness Phase 0

- Refactored to Core/DSP/UI layout; extracted `GainProcessor` effect module.
- Added Catch2 suite: unit tests, null test (golden ref), allocation gate.
- Added quality gate scripts, RT scanner, git hooks, GitHub Actions workflow.
- Verification: `./tools/run_dsp_tests.sh` (5 test cases, 521 assertions on null pass).
