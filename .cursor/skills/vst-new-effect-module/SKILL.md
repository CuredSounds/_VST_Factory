---
description: Scaffold a new JUCE effect module under Source/DSP/EffectModules/ with Catch2 test stub.
globs:
  - "**/Source/DSP/**"
  - "**/Tests/DSP_Tests/**"
---

# New Effect Module Skill

When the user asks to add a new effect module:

1. Create `Source/DSP/EffectModules/<Name>/` with `<Name>.h` and `<Name>.cpp`.
   - Pure C++ only — no JUCE GUI headers.
   - Pre-allocate in `prepare()`; `processBlock()` must not allocate.

2. Add `Tests/DSP_Tests/<Name>Tests.cpp` with at least one Catch2 `TEST_CASE`.

3. Register in CMake:
   - Add `.cpp` to `vst_factory_dsp` in `CMakeLists.txt`.
   - Wire from `Source/Core/PluginProcessor.cpp` (Core layer only).

4. Run before marking done:
   ```bash
   tools/run_build.sh && tools/run_dsp_tests.sh && tools/run_allocation_gate.sh
   ```

5. Reference: `_VST_Factory_Claude_docs/02_ARCHITECTURE_STANDARDS.md` §5.
