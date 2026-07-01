---
description: Regenerate null-test golden references after an intentional DSP sound change.
globs:
  - "**/Tests/**"
  - "**/Source/DSP/**"
---

# Null Test Golden Update Skill

Use only when the user **intentionally** changed DSP output.

1. Confirm the change is deliberate — null test failures are not auto-regressions.

2. Regenerate:
   ```bash
   cd vst_factory_sandbox
   UPDATE_GOLDENS=1 tools/run_null_tests.sh
   ```

3. Review the diff in `Tests/golden/` before committing.

4. Document in `CHANGELOG.md` with verification artifact path.

5. Never run `UPDATE_GOLDENS=1` in CI.
