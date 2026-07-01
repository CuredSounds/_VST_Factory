# Softysn1 — Code Review

**Date:** 2026-06-29
**Scope:** DSP / audio-thread real-time safety, architecture & maintainability, UI/editor code, performance hot paths.
**Codebase:** ~25,800 lines across 205 C++ files (JUCE VST, multi-part synth with Lua scripting host).

---

## Summary

This is a well-architected codebase. Cross-thread state uses `std::atomic`, `processBlock` opens with `ScopedNoDenormals`, there is a smart "skip render when idle" fast path, the look-and-feel is torn down correctly, and there is a clean central parameter-ID registry (`Softysn1::Params`). The synth voice and DSP modules are clean — no allocations or locks in the voice render path.

The problems that matter are concentrated in one place: the per-block parameter synchronisation at the top of `Source/Core/PluginProcessor.cpp::processBlock`. Fixing that single area removes the only real-time-safety hazards and most of the per-block CPU overhead.

Severity legend: **High** = can cause audible dropouts/glitches or undefined behaviour; **Medium** = maintainability / scalability; **Low** = polish.

---

## High — real-time safety in `processBlock`

### 1. ~190 heap allocations per audio block from string-built parameter IDs
The top of `processBlock` rebuilds `juce::String` parameter IDs every block to look parameters up:

- LFO custom-wave tables: 4 LFOs × 16 points (`PluginProcessor.cpp:284–294`)
- Mod-matrix slots: 16 × 4 fields, e.g. `"MOD" + juce::String(i) + "_SRC"` (`:395–402`)
- 12 macros (`:313–333`)
- 16 drum steps + 16 step probabilities (`:611–617`)
- 16 step-mod levels (`:652–656`)

Every `juce::String` concatenation allocates on the heap. That is roughly 190 allocations **per block**; at 48 kHz with 64-sample blocks (~750 blocks/sec) it is well over 100,000 heap allocations per second on the audio thread. Allocation on the audio thread takes a lock inside the allocator and is the classic cause of intermittent dropouts and glitches under load.

**Fix:** cache `std::atomic<float>*` pointers once (constructor or `prepareToPlay`) into fixed-size arrays, then index them in the audio thread — no strings, no map lookups, no allocations. Sketch:

```cpp
// header
std::array<std::atomic<float>*, kNumModSlots * 4>  modSlotParams {};
std::array<std::atomic<float>*, 4 * kLfoCustomPoints> lfoWaveParams {};
// ... macros, drum steps, step-mod, etc.

// prepareToPlay (or a cacheParameterPointers() helper called once)
for (int i = 0; i < kNumModSlots; ++i) {
    const auto p = "MOD" + juce::String(i);
    modSlotParams[i*4+0] = apvts.getRawParameterValue(p + "_SRC");
    modSlotParams[i*4+1] = apvts.getRawParameterValue(p + "_DEST");
    modSlotParams[i*4+2] = apvts.getRawParameterValue(p + "_AMT");
    modSlotParams[i*4+3] = apvts.getRawParameterValue(p + "_CURVE");
}

// processBlock — no string building
for (int i = 0; i < kNumModSlots; ++i) {
    activePatch().modSlots[i].source.store(modSlotParams[i*4+0]->load());
    activePatch().modSlots[i].dest  .store(modSlotParams[i*4+1]->load());
    activePatch().modSlots[i].amount.store(modSlotParams[i*4+2]->load());
    activePatch().modSlots[i].curve .store(modSlotParams[i*4+3]->load());
}
```

This is the single highest-value change in the codebase.

### 2. Two `juce::MidiBuffer` allocated per block
The velocity-remap block (`:539–556`) and the MPE filter block (`:558–598`) each construct a fresh `juce::MidiBuffer` every block and then `swapWith` it. `MidiBuffer` owns a heap buffer, so this allocates on the audio thread.

**Fix:** make them reusable members and call `.clear()` at the start of each block instead of constructing new ones. After warm-up the internal storage stops reallocating.

```cpp
// header
juce::MidiBuffer midiScratchA, midiScratchB;
// processBlock
midiScratchA.clear();
// ... fill midiScratchA ...
midiMessages.swapWith(midiScratchA);
```

### 3. File I/O on the audio thread
`applyDeferredPresetIfReady()` is called first thing in `processBlock` (`:144`). When an FX preset is pending it constructs an `FxPresetManager` and calls `loadPreset(deferredFxPresetFile)` — a disk read plus XML parse inside the audio callback (`:1808–1817`). It only fires when the user loads a preset, but it can stall an entire block and cause an audible gap.

**Fix:** parse the preset on the message/worker thread into a ready-to-apply structure, hand it to the audio thread via an atomic pointer / lock-free FIFO, and let the audio thread only perform the (cheap, non-allocating) swap.

### 4. The whole "copy every APVTS value into the matrix every block" pattern is O(hundreds) unconditionally
Beyond the string-built lookups, there are ~88 additional `getRawParameterValue` hash-map lookups per block (`:147–466`). Each is a map lookup that runs whether or not the value changed. Combined with #1 this is the bulk of the fixed per-block CPU cost.

**Fix:** the pointer-caching from #1 covers correctness/allocation. To also cut the redundant work, consider driving the matrix from APVTS parameter listeners (write-on-change) instead of polling all parameters every block, or at minimum cache all the pointers and skip the map lookups. Most of these are GUI-rate parameters that do not need per-block polling.

---

## Medium — architecture & maintainability

### 5. `PluginProcessor.cpp` is 1,820 lines
About 260 lines are the mechanical per-block APVTS→matrix copy and ~500 lines are `createParameterLayout()`. Both are mechanical and would move cleanly into dedicated helpers (e.g. an `ApvtsCache` / `ParameterBlock` type and a `ParameterLayout` builder file). This shrinks the processor, isolates the hot path so it is reviewable on its own, and naturally pushes you toward the pointer-caching in #1.

### 6. Minor dead code (not the forwarding shims)
The top-level `Source/PluginProcessor.h`, `Source/PresetManager.h`, `Source/PluginEditor.h`, `Source/VoiceParameterMatrix.h`, and `Source/UI/BioSynthLookAndFeel.h` are **intentional one-line forwarding includes**, not duplicates — leave them. Genuinely unreferenced files are only:

- `Source/UI/UIHelpers.h`
- `Source/DSP/Multi/VoiceMatrixCopy.h`
- `Source/DSP/Multi/LegacyPartBridge.h`
- `Source/DSP/LayerStack.h`
- `Source/SynthEditorContent.cpp` (481 lines; not in the CMake build)

Worth confirming and removing to reduce noise, but low impact.

---

## Low — polish

### 7. `juce::Random::getSystemRandom()` in the audio thread
Used at `:713` for the sample-and-hold fallback. `getSystemRandom()` returns a shared instance; prefer a per-processor member `juce::Random` to avoid any shared-state contention.

### 8. Master-gain read/write inconsistency
Master gain is written from the `MASTER_GAIN` parameter into `activePatch().masterGain` (`:161`) but read back from `multiPatch.masterGain` (`:694`) when applying gain. These are two different storage locations; verify they are intentionally kept in sync, otherwise the `MASTER_GAIN` knob and the applied gain can diverge.

---

## What looks good (keep doing this)

- `ScopedNoDenormals` at the top of `processBlock`.
- All cross-thread synth state goes through `std::atomic`.
- The `skipSynthRender` idle fast path avoids running the full graph when nothing is sounding.
- Voice render path (`SynthVoice.cpp`) and DSP/effect modules are allocation- and lock-free.
- UI: look-and-feel is set and cleared in pairs; no risky `MessageManager::callAsync([this]...)` captures spotted; timers are stopped.
- Central `Softysn1::Params` ID registry instead of scattered string literals.

---

## Suggested order of work

1. **#1 (cache parameter pointers)** — removes most allocations and most per-block CPU. Highest value.
2. **#2 (reusable MIDI scratch buffers)** — small, low-risk, removes the remaining per-block allocations.
3. **#3 (preset I/O off the audio thread)** — removes the last real-time hazard.
4. **#5 (split the processor)** — makes the above easier to maintain and review.
5. **#6–#8** — cleanup when convenient.

All four high-priority items live in `Source/Core/PluginProcessor.{h,cpp}`, so they can be done in one focused pass.
