## Summary

<!-- What changed and why -->

## Verification artifacts

<!-- Required for any "done" claim — link test paths, CI run, or profiler output -->

- [ ] `tools/run_build.sh` passes
- [ ] `tools/run_dsp_tests.sh` passes
- [ ] `tools/run_allocation_gate.sh` passes (if DSP/audio thread touched)
- [ ] `tools/run_null_tests.sh` passes (if DSP touched)
- [ ] RT scan clean: `python3 vst_factory_sandbox/tools/scan_rt_violations.py`

## Test plan

- [ ] Local pre-push hooks pass (`git config core.hooksPath .githooks`)
