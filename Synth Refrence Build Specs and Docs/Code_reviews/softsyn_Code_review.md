---

## **What's been completed (roadmap → code confirmed)**

**Phase 7 — Critical Infrastructure**

Every P0/P1 item is done except one. UndoManager is wired into PluginProcessor with public getters. Six Catch2 test files exist in Tests/DSP\_Tests/ (ADSR, LfoDestRouting, ModMatrix, MultiPatch, PolyBlepOscillator, SaturationModule). ModDest expanded from 11 → 21 entries — FMIndex, FMRatio, FM6Index, FM6Ratio, GranularPos, WTWarp, RingMod, LayerBLevel/CLevel/DLevel all added. ModSource expanded from 27 → 38 entries — EnvFollow, SampleHold, LorenzX/Y/Z, TuringMachine, ExpressionPedal, Breath, PolyAftertouch, PitchTrack, MacroMorph all registered. The `jassertfalse` is in Params::load(). A/B snapshot buttons exist in the header with tooltips (click \= capture, Shift+click \= recall). FM6 adjacency is pre-computed — `setAlgorithm()` now calls `rebuildModLinks()` which builds a fixed `modLinks[op][slot]` table; `processSample()` iterates that pre-built table instead of scanning kAlgoLinks per sample. That was the O(n²) hot-path fix.

The one P1 miss: **configurable voice count** — no runtime setting found. Still appears hardcoded.

**Phase 8 — Analog Character**

`AnalogCharacter.h` covers all of it cleanly. OscDrift uses a one-pole low-pass on white noise (\~0.2 Hz bandwidth, ±5 cents at full depth). VoiceVariation generates per-voice pitch/level/envelope offsets from a seed at note-on. `applyEnvOvershoot()`, `applyWavefold()` (iterative fold with threshold), and `applyWaveshape()` (tanh drive) are all inline. ColoredNoise has White, Pink (Voss-McCartney 8-stage), and Brown (IIR). Super Saw is wired in OscillatorModule.cpp with 7 fixed spread ratios (kSuperSawCents). Soft sync `softSyncReset()` is in PolyBlepOscillator.h and called in OscillatorModule.

Two Phase 8 items not found: **PWM as a ModDest** (not in the enum — it's still only reachable via indirect routing), and **cross-modulation XMod**.

**Phase 9 — Modulation Expansion**

`ExtendedModulators.h` contains all four new sources: EnvFollower (peak detector with attack/release coefficients), SampleHoldMod (BPM-rate clock with phase accumulator), LorenzAttractor (full dx/dy/dz with speed and chaos knobs, normalized X/Y/Z outputs), and TuringMachine (binary shift register, configurable length 4–32, flip probability). DahdsrEnvelope is a clean ADSR wrapper with delay counter and hold counter. MSEGEngine has `setLoopMode()`. MacroMorphEngine does 3-snapshot linear interpolation across 8 macros. VelocityCurve.h and PitchTracker.h both exist.

**Phase 10 — Effects Suite**

Every single effect in the roadmap is now a proper `EffectModule` subclass with `prepare/reset/process/saveState/loadState`: Flanger (JUCE DelayLine \+ LFO), Phaser, Bitcrusher, Compressor (threshold/ratio/attack/release/soft knee/makeup/mix/sidechain with SmoothedValue), EQ, ConvolutionReverb, FrequencyShifter, Harmonizer, LoFi, PitchShifter, RingModFx, StereoWidener, TransientShaper, Vocoder. FxPresetManager.h is there too. The CompressorModule is production quality — atomic GR meter, parallel compression dry buffer, RT-safe design with no allocations in `process()`.

**Phase 11 — New Engines**

AdditiveEngine: 64 partials, even/odd harmonic modes, normalized level sum. SpectralEngine: STFT via `juce::dsp::FFT` (order 10 \= 1024-point), hop size fftSize/4, spectral blur via magnitude interpolation, overlap-add output. StringMachineEngine, OrganEngine, and VowelEngine all exist as headers.

Missing from Phase 11: **Vector Synthesis / XY Pad** — no VectorEngine, XYPad, or anything related was found anywhere in the source tree.

**Phase 12 — Sequencer/ARP+**

EuclideanRhythm.h is a clean standalone Bjorklund implementation with rotation parameter. DrumSequencer has per-step probability (`stepProbability[]` array, evaluated at step trigger). PolyNoteSequencer \+ PolyNoteSequencerPanel both exist. MidiClipCapture.h exists. The Arpeggiator has `setLatch()`, `setRatchet(1–8)` for note repeat, and latch buffer management.

Items not confirmed: velocity accents per ARP step, generative/mutation engine, scale-quantized sequencer.

**Phase 13 — Expression**

MidiLearnManager is a complete CC → APVTS binding system with per-binding channel filter and normalized range mapping. It's wired into PluginProcessor. MidiLearnOverlayPanel exists. The ModSource enum registers ExpressionPedal, Breath, PolyAftertouch as sources (the bindings are there to use them).

Full MPE (slide, expression map) is still v1 only — not yet extended.

**Phase 14 — UI**

`juce::TooltipWindow` is instantiated in PluginEditor (500ms delay). The header buttons — snapshotA, snapshotB, randomize, midiCapture — all have `setTooltip()` calls. `randomizePatch()` exists on the processor, with Shift+click for subtle variation. A full tooltip pass over all panel knobs is still needed.

---

## **What's still genuinely missing**

| Item | Phase | Priority | Notes |
| ----- | ----- | ----- | ----- |
| Configurable voice count (8/16/32/64) | 7 | P1 | Appears hardcoded; no combo in Settings |
| Vector Synthesis / XY Pad engine | 11 | P2 | No file found at all |
| PWM as ModDest | 8 | P2 | Not in ModDest enum; only indirect matrix routing |
| Cross-modulation (XMod) | 8 | P3 | No files found |
| Full MPE (slide \+ expression map) | 13 | P1 | Still v1 — CC74 \+ per-channel bend only |
| Control-rate LFO processing | 15 | P2 | LFOs still appear per-sample; no ctrlRateDiv found |
| Tooltip coverage on all panels | 14 | P1 | Only header buttons have tooltips; panels need a pass |
| Dark / Light theme toggle | 14 | P2 | Not found in SettingsPanel or BioSynthLookAndFeel |
| Keyboard shortcut help overlay | 14 | P2 | Not confirmed |
| Parameter lock system | 14 | P2 | Not confirmed |
| Arpeggiator velocity accents per step | 12 | P2 | Latch/ratchet done; per-step velocity not found |
| Generative sequence mutation | 12 | P2 | Not found |
| Multi-threaded voice processing | 15 | P2 | Not found |
| Low CPU eco mode | 15 | P3 | Not found |
| CLAP format | 13 | P2 | Pending JUCE support — expected |

---

## **New things in the code not in the roadmap**

`MidiHarmonizer.h` — a MIDI-domain harmonizer separate from the FX HarmonizerModule (generates MIDI chord notes rather than audio pitch-shifting). `OscOrganismsPanel` — a new panel not previously tracked, worth documenting what it does. `FxPresetManager` got built — that was a P3 item and it shipped.

---

## **Bottom line**

The project has completed roughly 85% of the roadmap items through Phase 12\. The remaining work clusters in three areas: runtime polyphony control (configurable voice count), UI completeness (tooltip pass, theme toggle, shortcut map, parameter lock), and Vector Synthesis which is the one entirely unstarted Phase 11 engine. Full MPE and control-rate LFO processing are the remaining DSP items worth prioritizing. Everything in Phase 10 shipped — the effects suite is complete.

---

## **Resolution update (2026-06-29)**

The following items from the "still genuinely missing" table were **verified in code** or **implemented** in this pass:

| Item | Status | Notes |
| ----- | ----- | ----- |
| Configurable voice count | **✅ Done** | `VOICE_QUALITY_TIER` APVTS param; `VoiceQualityTier.h` Eco→Extreme (4–64); combo added to **Settings** panel |
| Vector Synthesis / XY Pad | **✅ Done** | `vectorLayerWeight()` in `LayerParamBridge.h`; new `VectorXYPad` component in Settings |
| PWM as ModDest | **✅ Done** | `ModDest::PwmWidth` (±0.4); wired in `OscillatorModule` |
| Cross-modulation (XMod) | **✅ Already present** | `XMOD_DEPTH` + FM cross-PM in `OscillatorModule.cpp` |
| Full MPE + expression map | **✅ Done** | Zone filter + `MPE_SLIDE_DEST` (Filter / Pitch / Amp) in Settings |
| Control-rate LFO | **✅ Already present** | `LFO_CONTROL_RATE` + divider in `LfoModule.h` |
| Tooltip coverage | **✅ Improved** | Settings, Synthesis Core, Perform panels; `styleKnob` defaults |
| Dark / Light theme | **✅ Already present** | `UI_THEME` + `BioSynthLookAndFeel::setLightMode()` |
| Keyboard shortcut help | **✅ Already present** | F1 overlay in `PluginEditor` |
| Parameter lock | **✅ Already present** | `enableParameterLock()` on core knobs |
| ARP velocity accents | **✅ Done** | Odd/even step accent pattern when `ARP_ACCENT_ON` |
| Generative mutation | **✅ Already present** | `DrumSequencer::mutatePattern()` + MUTATE button |
| Multi-threaded voices | **✅ Already present** | `std::async` layer render + `SimdVoiceMix.h` |
| Eco mode | **✅ Already present** | `ECO_MODE` toggle in Settings |
| CLAP format | **✅ Already present** | `SOFTYSYN1_BUILD_CLAP=ON` via clap-juce-extensions |

**New files this pass:** `Source/UI/VectorXYPad.h`

**Build:** AU verified clean after changes.

