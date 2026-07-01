# Softysn1 — Feature Implementation Plan

**Date:** 2026-06-29
**Covers:** contextual per-engine osc controls, resizable scopes, CPU reduction, the Ableton exit crash, a modulation-overview window, and a full sampler editor (Ableton/Bitwig-style).

Each section gives the problem, where it lives in the current code, a recommended approach, and the concrete steps. Ordered by suggested priority.

---

## 1. Ableton exit crash (do this first — it's a real bug)

### Most likely cause
`Source/UI/BioSynthFonts.cpp:9–11` holds three embedded typefaces in **namespace-scope (global) storage**:

```cpp
namespace {
    juce::Typeface::Ptr displayFace;
    juce::Typeface::Ptr monoRegularFace;
    juce::Typeface::Ptr monoBoldFace;
}
```

These are reference-counted JUCE objects that are never released until C++ static destruction, which on plugin unload runs **after** Ableton has begun tearing down JUCE's graphics/font subsystem. Releasing a `Typeface` at that point touches already-destroyed global state and crashes. This is one of the best-documented "crashes only on exit" patterns in JUCE plugins, and it matches your symptom exactly (only the synth loaded, crash on quit, not during use).

### Fix (preferred): tie font lifetime to the editor via `SharedResourcePointer`
Make the typefaces members of a small holder owned through `juce::SharedResourcePointer`, so they're created when the first editor opens and destroyed when the last editor closes — always while JUCE is still alive.

```cpp
// BioSynthFonts.h
struct FontBank {
    juce::Typeface::Ptr display, monoRegular, monoBold;
    FontBank();                 // load from BinaryData here
};

// usage: hold one in the editor
juce::SharedResourcePointer<FontBank> fonts;   // member of the editor
```

Have `displayFont()` / `monoFont()` take the bank (or read it from a `SharedResourcePointer` they construct locally — the resource stays alive as long as the editor's copy does).

### Fix (minimal): explicitly release on editor close
If you'd rather not refactor, add `BioSynthFonts::releaseFonts()` that nulls the three `Ptr`s, and call it from `Softysn1AudioProcessorEditor::~Softysn1AudioProcessorEditor()` (alongside the existing `setLookAndFeel(nullptr)`). That guarantees release happens on the message thread while JUCE is alive. The risk: if another editor instance is open it would drop the cache early — `SharedResourcePointer` avoids that, so prefer the first option.

### Verify
The real proof is a stack trace. In Ableton, run a Debug build of the plugin and check the crash backtrace points into `Typeface`/`CoreText`/`juce::Font` teardown. Also worth ruling out as secondary suspects:
- `juce::dsp::Convolution` in `ConvolutionReverbModule` spawns a background thread; confirm the module (and so the Convolution) is destroyed before the processor finishes tearing down. It normally self-joins, but verify there's no dangling reference to processor state.
- The `WeakReference<Softysn1AudioProcessor>` + `callAsync` path in `loadFxPreset` — fine as long as `JUCE_DECLARE_WEAK_REFERENCEABLE` is present and a queued lambda can't outlive the message manager. Low risk, but check.

---

## 2. Contextual per-engine oscillator controls

### Current state
`OscColumnHub` already declares every control for every engine as a member and toggles a subset with `setVisible()` in a 12 Hz `timerCallback` (`OscColumnHub.cpp:153–177`) — but only for Wavetable / 2-op FM / FM6. The other engines in your 15-type list (Sampler, Granular, Karplus, Modal, Additive, Spectral, String, Vowel, Organ — `OscTypeChoices.h`) have their controls scattered across other panels. That's exactly the "static controls in random spots" problem.

### Recommended approach: an engine-editor swap zone
Treat the area directly under the OSC A selector as a single **engine editor host** that shows exactly one child component — the editor for the currently selected engine.

1. Define one small `Component` subclass per engine (`WavetableOscEditor`, `Fm6OscEditor`, `GranularOscEditor`, `SamplerOscEditor`, `KarplusOscEditor`, …). Each owns only its own sliders/combos and their APVTS attachments, and implements its own `resized()`. Several already exist as loose control groups elsewhere — moving them into dedicated editors is mostly cut-and-paste.
2. In the host, keep them in `std::array<std::unique_ptr<juce::Component>, kNumOscTypes>` (or a map). Show one at a time:
   ```cpp
   void setEngine(int oscType) {
       if (oscType == current) return;
       if (auto* prev = editors[current].get()) prev->setVisible(false);
       current = oscType;
       auto* next = editors[current].get();
       addAndMakeVisible(*next);
       next->setBounds(engineZone);   // engineZone computed in resized()
   }
   ```
3. **Drive it from the parameter, not the timer.** Use an `AudioProcessorValueTreeState::Listener` (or `ParameterAttachment`) on `OSC_TYPE` so the swap happens exactly when the engine changes, on the message thread. This also removes per-frame `setVisible` churn. Keep a timer only for the live scope animation.
4. Above the zone, host one **scope view** that itself switches representation by engine (wavetable stack for WT, operator/algorithm diagram for FM6, sample thumbnail for Sampler, harmonic bars for Additive, etc.). You already have building blocks in `WaveformDisplay.h` (`CycleScope`, `WavetableStackView`, `OscShapeView`).

### Why this is worth it
It localizes each engine's UI to one file, kills the "where did that knob go" problem, and makes adding a new engine a self-contained task (new editor + register it). It also shrinks `OscColumnHub` substantially.

### Migration path (low-risk, incremental)
- Step 1: extract the **existing** WT / FM / FM6 controls into three editor components and host-swap them — behavior identical, but now structured.
- Step 2: move one scattered engine at a time (start with Granular or Sampler) into its own editor.
- Step 3: delete the now-empty control groups from the other panels.

---

## 3. Resizable scopes / using the empty master-scope area

Two separate asks here.

### a) Resize panels with +/- keystrokes
The editor already handles keys in `Softysn1AudioProcessorEditor::keyPressed` (`PluginEditor.cpp:252`). Add a "focused scope" concept:
- Track which scope is under the mouse or last clicked (`activeScopeId`).
- On `'='`/`'+'` and `'-'`, adjust a stored height/zoom factor for that scope and call `resized()`.
- Persist the factor as a non-automatable APVTS value or in the editor state so it survives reopen.

A cleaner alternative that DAW users expect: make the scope area a draggable splitter (JUCE `StretchableLayoutManager` + `StretchableLayoutResizerBar`). Keystrokes can still nudge the same layout value. I'd offer both — splitter for discoverability, keys for speed.

### b) Fill the large empty master-scope region
That space is ideal for the per-engine scope from §2 and/or the modulation-overview from §5. Rather than a single static master oscilloscope, make it a **switchable visual bay**: tabs or a small selector toggling between {Master output scope, Active engine scope, Mod overview, Spectrum}. Reuses the scope components you already have and gives the empty area a purpose.

---

## 4. CPU reduction

The biggest wins overlap with the prior code review (see `Claude_softsyn_review_2026-06-29.md`): the per-block APVTS→matrix copy in `processBlock` does ~190 heap allocations and ~88 map lookups **every block, unconditionally**. Fixing that (cache `std::atomic<float>*` pointers; reuse MIDI scratch buffers) is the single largest CPU + glitch win. Beyond that:

- **Make parameter→matrix sync change-driven.** Most of those parameters are GUI-rate. Drive the matrix from APVTS parameter listeners (write-on-change) instead of polling all of them per block. Eliminates almost all of the fixed per-block cost.
- **Control-rate modulation.** You already have `LFO_CONTROL_RATE` and `ECO_MODE`. Ensure LFOs/MSEGs/slow mod sources update at block or sub-block rate (e.g. every 16–32 samples) rather than per-sample when in non-audio-rate modes. Confirm the granular `maxGrains` cap and voice tiers actually gate work.
- **Skip silent parts/effects.** `processBlock` already has `skipSynthRender` and `partAudible` — extend the idea to per-effect bypass: an effect at default/zero mix shouldn't run its inner loop. Check the convolution reverb and oversampled effects especially; oversampling is often the dominant cost.
- **Voice stealing / max polyphony.** Make sure idle voices fully short-circuit (the voice path looked clean, but verify `renderNextBlock` returns early when the amp envelope is finished rather than processing zeros through filters).
- **Denormals.** `ScopedNoDenormals` is set globally — good; just confirm feedback paths (delays, combs, Karplus) flush-to-zero so they don't drop into denormal territory between notes.

Recommend profiling with a single sustained patch per engine to attribute cost; the telemetry you already compute (`getCpuPercent`) plus a sampling profiler (Instruments / VTune) will point at the real hot engines quickly.

---

## 5. Modulation-overview window (click a scope → see all sources/destinations)

### Approach
A modal-less `juce::DocumentWindow` (or an in-editor overlay like the existing `midiLearnOverlay`) that renders:
- All LFOs / MSEGs / envelopes as live animated curves (reuse `LfoShapeView`, `MsegGraphEditor`, `AdsrGraphEditor`).
- The mod-matrix routing as a source→destination list or node graph, read from the 16 mod slots (`activePatch().modSlots[i]` — source/dest/amount/curve, already atomics).
- Text overlays naming each route, using your existing `getModSourceNames()` / `getModDestNames()`.

### Wiring
- Add a click handler on the scope components (`mouseDown`) that calls an editor callback to open/toggle the overview.
- The overview reads matrix state at ~30 Hz via a timer (all atomics, so no locking) and repaints.
- For "show overlays of which source hits which destination," highlight a route on hover and draw a connector line to the corresponding control if it's on-screen.

Start read-only (visualization) — it's genuinely useful on its own — then optionally allow editing routes from the same view later.

---

## 6. Sampler editor (Ableton/Bitwig-style)

### Current state
- DSP (`SamplerModule.h`) supports start/end, loop start/end, 3 loop modes, root note, linear interpolation, 4 key zones. It has **no gain, no normalize, no slice playback**.
- UI (`SampleMapPanel`) is sliders + a "SLICE SCAN" button and a slice-count label; **no waveform display, no trim/loop handles, no slice markers**.
- The processor owns the audio (`sampleZoneBuffers`, `juce::AudioBuffer<float>`) and exposes `estimateSliceCountForZone`.

### a) Waveform viewer with trim + slices
Use `juce::AudioThumbnail` built from the zone buffer (it's designed exactly for this and is cheap to repaint):
- A `SampleWaveformView` component that draws the thumbnail, plus draggable handles for start/end and loop start/end, mapped to the existing `SMPL_START` / `SMPL_END` / `SMPL_LOOP_START` / `SMPL_LOOP_END` params via `ParameterAttachment`.
- Draw slice markers as vertical lines at the detected slice positions; clicking a marker can audition or set a play range.
- Drag handles write to the params; the DSP already reads them, so playback follows immediately.

### b) Normalize and gain
These need new state:
- **Gain:** add an `AudioParameterFloat` `SMPL_GAIN` (e.g. −24…+24 dB) and apply it in `SamplerModule::getNextSample` (multiply `out` by a linear gain alongside `velGain`). Trivial DSP change, fully automatable.
- **Normalize:** a non-realtime, message-thread operation. Scan the loaded zone buffer for peak, compute the gain that brings peak to 0 dBFS (or a target), and either (i) apply it destructively to the stored buffer, or (ii) store it as a per-zone offset combined with `SMPL_GAIN`. Prefer non-destructive (store a `normalizeGain` per zone) so it's reversible. Add a "Normalize" button per zone in the panel.

### c) Slices
You already estimate slice counts. To make slices playable like Ableton's Simpler "Slice" mode:
- Store slice boundary positions per zone (transient-detected or evenly divided).
- Add a slice playback mode where incoming MIDI notes map chromatically to slices (note → slice index → start/end range).
- Visualize boundaries in the waveform view; allow drag to adjust, click to add/remove.

### Matching Ableton/Bitwig feel
The expected interactions: drag waveform edges to trim, shift-drag for loop region, double-click to zoom to selection, scroll to zoom, snap handles to zero-crossings (avoids clicks), and a clear visual distinction between play range (start/end) and loop range. Zero-crossing snap is a small helper that searches outward from the dragged sample for a sign change — cheap and a big quality boost.

### Suggested order
1. `SMPL_GAIN` param + apply in DSP (smallest, immediately useful).
2. `AudioThumbnail` waveform view with draggable start/end/loop handles.
3. Normalize button (non-destructive per-zone gain).
4. Slice markers → slice playback mode.

---

## Recommended overall sequence

1. **Exit crash** (§1) — small, removes a shipping-blocker.
2. **CPU / per-block sync fix** (§4 + the prior review) — biggest stability/perf win, also makes everything else smoother.
3. **Sampler gain + waveform/trim view** (§6a–b) — high user value, self-contained.
4. **Contextual engine controls** (§2) — the largest UI refactor; do it incrementally.
5. **Switchable visual bay + resize** (§3) and **mod overview** (§5) — build on the components from §2.

All of these are additive and can land independently; none requires a parameter-state break except adding `SMPL_GAIN` (append it to the layout so existing presets still load).
