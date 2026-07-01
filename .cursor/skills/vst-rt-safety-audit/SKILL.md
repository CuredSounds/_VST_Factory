---
description: Run real-time safety audit on a diff or module — report like Softsyn1 code reviews.
globs:
  - "**/Source/**"
  - "**/Tests/**"
---

# RT Safety Audit Skill

When auditing audio-thread safety:

1. Run `python3 vst_factory_sandbox/tools/scan_rt_violations.py`.

2. Manually check `processBlock` and callees for:
   - `juce::String` construction or concatenation
   - `std::async`, `std::thread`, mutex locks
   - `std::vector::push_back` / resize on audio thread
   - Raw string parameter IDs outside `Parameters.h`

3. Output report format:
   ```
   ## RT Safety Audit — <module>
   ### P0 (ship-blocking)
   - ...
   ### P1
   - ...
   ### Verified clean
   - ...
   ```

4. Reference catalog: `_VST_Factory_Claude_docs/03_GUARDRAILS_AND_RT_SAFETY.md`

5. For changes touching voice allocation or signal flow, recommend ensemble review
   per `_VST_Factory_Claude_docs/04_TESTING_AND_QUALITY_GATES.md` §6.
