# DSP Tests

Catch2 tests for `Source/DSP/` — run via:

```bash
tools/run_build.sh
tools/run_dsp_tests.sh
```

| Tag | File | Purpose |
|-----|------|---------|
| `[dsp][gain]` | GainProcessorTests.cpp | Unit tests |
| `[dsp][null]` | NullTest.cpp | Golden reference diff |
| `[rt][allocation]` | AllocationTest.cpp | Zero heap allocs in processBlock |

Update goldens: `UPDATE_GOLDENS=1 tools/run_null_tests.sh`
