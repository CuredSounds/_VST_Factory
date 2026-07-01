# Changelog

> **Reference only.** Softsyn1 history. Sandbox harness changes: [`vst_factory_sandbox/CHANGELOG.md`](vst_factory_sandbox/CHANGELOG.md).

All notable changes to **Softysn1 / Bio-Synth**. Format: `[YYYY-MM-DD] Name: summary`.

---

## [2026-06-29] JUCE FetchContent migration

- Removed vendored `JUCE-master/` from the repo (~92 MB).
- CMake now fetches **JUCE 8.0.12** via `FetchContent` (`cmake-build-*/_deps/juce-src`).
- **clap-juce-extensions** pinned to `16e9d4c` (JUCE 8.0.11+ headless compatibility).
- `JUCE-master/` added to `.gitignore` for leftover local checkouts.

---

## [2026-06-29] Factory preset library refresh (68 patches)

- **All 56 existing presets** merged with new APVTS params (analog character, vector blend, MPE, ARP+, modulators, etc.).
- **12 new showcase presets** for Additive, String, Organ, Vector VS, Wavefold, XMod FM6, Analog Drift, SuperSaw, ARP ratchet, Lorenz chaos, pink noise, env-follow sweep.
- **13 targeted updates** (e.g. WT_Spectral_Pad → Spectral engine, Formant → Vowel osc, ARP accent/latch/ratchet).
- Script: `scripts/refresh_factory_presets.py` for future param schema bumps.

---

- **PWM ModDest:** `ModDest::PwmWidth` added; matrix routes to square duty in `OscillatorModule`.
- **Vector XY Pad:** New `VectorXYPad` component in Settings (replaces X/Y sliders).
- **Voice quality in Settings:** `VOICE_QUALITY_TIER` combo (Eco 4 → Extreme 64 voices/part).
- **MPE expression map:** `MPE_SLIDE_DEST` — CC74 routes to Filter Cutoff, Pitch, or Amp per voice.
- **ARP accents:** Odd/even step velocity alternation when `ARP_ACCENT_ON` enabled.
- **Tooltips:** Expanded pass on Settings, Synthesis Core, and Perform panels.

---

## [2026-06-29] Flagship Roadmap — Phases 7–15 COMPLETE (session 3)

### New Engines (Phase 11)
- **Spectral STFT** (osc 11), **String Machine** (12), **Vowel/Formant** (13), **Organ/Tonewheel** (14).
- **Wavefolder / Waveshaper / XMod** on VA + FM paths (`WAVE_FOLD`, `WAVESHAPE`, `XMOD_DEPTH`).

### FX (Phase 10)
- **Harmonizer, Convolution Reverb, Vocoder, Ring Mod FX, Frequency Shifter** in effect factory.
- **FX Preset System** via `FxPresetManager` (deferred audio-thread load).

### Modulation (Phase 9 P3)
- **Pitch Tracker** (`PitchTracker` → `ModSource::PitchTrack`).
- **Macro Morph** (`MacroMorphEngine` → `ModSource::MacroMorph`).

### Sequencer (Phase 12)
- **Poly note sequencer** panel + `PolyNoteSequencer` (16×4 notes).
- **Drum mutation** (`mutatePattern` + MUTATE button).
- **Scale-quantized poly seq** wired to `SCALE_ON/ROOT/MODE`.
- **MIDI clip capture** (header MIDI CAP → `.mid` export).

### Expression (Phase 13)
- **Full MPE:** zone bounds, per-channel CC74 slide, per-note poly AT.
- **MPE Zone Manager** in Settings (`MPE_ZONE_LOWER/UPPER`).
- **CLAP format** via clap-juce-extensions FetchContent.

### UI / UX (Phase 14)
- **Preset 1–5 star ratings** in patch browser + min-rating filter.
- **Parameter lock UI** (`enableParameterLock` — right-click padlock).
- **Layer copy/paste** context menu.
- **FX waveform previews** for all effect types.
- **Animated mod viz:** mod-source hover labels on knobs.

### Performance (Phase 15)
- **SIMD voice mix** (`SimdVoiceMix.h`).
- **Multi-threaded layer render** (`std::async` in `SynthVoice`).
- **OpenGL output scope** with CPU fallback.
- **Deferred preset load** (`requestDeferredLoad` — audio-thread swap).

### Build
- **AU + CLAP** verified; installed to system plugin folders.

### Roadmap
- `Softysyn1_Flagship_Roadmap.md` — all phases signed off ✅ (AAX ship remains partial — requires Avid SDK).

---

## [2026-06-29] Flagship Roadmap — Phases 8–15 completion batch (session 2)

### DSP / Engines
- **Pink/Brown noise:** `ColoredNoise` wired in `OscillatorModule` via `NOISE_COLOR`.
- **DAHDSR:** `DahdsrEnvelope` on amp + filter env (`AMP_ENV_DELAY/HOLD`, `FILTER_ENV_DELAY/HOLD`).
- **Velocity curve:** `VelocityCurve` presets + MIDI velocity remapping in `processBlock`.
- **Additive engine:** Osc type 11 (`AdditiveEngine`, 64 partials, `ADDITIVE_BRIGHTNESS`).
- **Vector synthesis:** `VECTOR_BLEND_X/Y` with Prophet VS-style layer weights.
- **Control-rate LFOs:** `LFO_CONTROL_RATE` divider in `LfoModule`.
- **Eco mode:** `ECO_MODE` caps voices, LFO rate, granular grains (8).
- **Granular:** `setMaxActiveGrains()` for spawn limiting.

### FX (Phase 10 P2)
- **Stereo Widener, Transient Shaper, Lo-Fi, Pitch Shifter** registered in `EffectModuleFactory` + CMake.

### Arpeggiator (Phase 12)
- **Latch, ratchet, step velocity accents:** `ARP_LATCH`, `ARP_RATCHET`, `ARP_STEP_VEL`, `ARP_ACCENT_ON` + Perform panel UI.

### UI / UX (Phase 14)
- **Dark/Light theme:** `BioSynthLookAndFeel::setLightMode()` + Settings theme combo.
- **MIDI Learn overlay:** `MidiLearnOverlayPanel` (Cmd+L).
- **Keyboard shortcuts:** F1 help dialog.
- **Parameter lock:** API on processor; randomizer respects locks.

### Build
- AU build verified and installed to `~/Library/Audio/Plug-Ins/Components/`.

---

## [2026-06-29] Flagship Roadmap — Phases 7–14 implementation batch

### Phase 7 — Critical Infrastructure
- **Undo/Redo:** Complete (`UndoManager` + Cmd+Z / Cmd+Shift+Z + header undo status label).
- **FM6 adjacency:** Pre-computed `modLinks` at `setAlgorithm()` (was already in place; roadmap marked done).
- **Voice count:** `Extreme (64/part)` tier added to `VOICE_QUALITY_TIER`.
- **A/B patch snapshot:** Header A/B buttons — click captures, Shift+click recalls.

### Phase 8 — Analog Character
- **Oscillator drift, voice spread, filter saturation, PWM, soft sync, super saw, env overshoot** — DSP + APVTS params wired (`AnalogCharacter.h`, `OscillatorModule`, `SynthVoice`).

### Phase 9 — Modulation Expansion
- **New mod sources:** Env Follow, Sample & Hold, Lorenz X/Y/Z, Turing Machine, Expression (CC11), Breath (CC2), Poly Aftertouch.
- **MSEG loop** was already implemented.

### Phase 10 — Effects Suite
- **Flanger, Phaser, Bitcrusher** effect modules registered in `EffectModuleFactory` (EQ + Compressor were already present).

### Phase 12 — Sequencer
- **Euclidean rhythm** generator (`EuclideanRhythm.h`) applied to drum sequencer from `DRUM_EUC_*` params.
- **Per-step probability** (`DRUM_STEP_PROB_0..15`) with random gate per step.

### Phase 13 — Expression (partial)
- **MIDI Learn** infrastructure (`MidiLearnManager`) + CC11/CC2 global routing.
- Full MPE zone manager, CLAP, poly-aftertouch UI — still open.

### Phase 14 — UI Polish (partial)
- **Patch randomizer** (RND button, Shift = subtle).
- **Undo visual feedback** in header.
- **Tooltips** on A/B/RND buttons; comprehensive tooltip pass still open.
- **UI_THEME** param scaffolded (dark/light toggle UI TBD).


- **`ModularFXRack`:** Dry pass-through when no lanes contribute (fixes `masterBusFx` wiping the mix).
- **Phase 7 start:** `UndoManager` on APVTS (64 steps); Cmd+Z / Cmd+Shift+Z in plugin editor.

## [2026-06-27] Omnisphere Multi — Sprint 11+ (Phase 3 complete)

- **Per-part editor scope:** `PartPatchBank` stashes/restores APVTS + FX + ARP per part; `switchActivePart()` on part bar.
- **Per-part FX/ARP:** `std::array<ModularFXRack, 8>` + `std::array<Arpeggiator, 8>` processed in `MultiEngine::render`; `masterBusFx` post-mix.
- **Layer signal path:** `P{n}_SIGPATH` — shared filter, per-layer filters, or serial layers; layer `FLT_*` params in `renderLayerAudio`.
- **Multi presets:** `MultiPresetManager` saves/loads `.softmulti` (8-part bank + master gain + voice tier); UI in `MultiMixerPanel`.
- **UI:** `LayerTabsPanel` — A/B/C/D focus, signal path combo, per-layer filter strip (scoped to active part).
- **Build:** VST3 green (`SOFTYSN1_BUILD_TESTS=OFF`).

## [2026-06-27] Omnisphere Multi — Sprint 8–10 (8 Parts + A/B/C/D full osc)

- **4-layer voice engine:** `SynthVoice` renders A/B/C/D with identical osc parity (VA, WT, Sampler, Granular, Karplus, Modal, FM6) via `renderLayerAudio` + `LayerParamBridge`.
- **8-part audio:** `MultiEngine` replaces single `juce::Synthesiser`; MIDI routed per part (`PartSynthSound` channel + key range).
- **Voice tiers:** `VOICE_QUALITY_TIER` — Eco 4 / Mid 8 / High 16 / Ultra 32 voices per part.
- **APVTS:** `P{n}_*`, `P{n}_LAY{A|B|C|D}_*`, `MULTI_ACTIVE_PART`, `MULTI_MASTER_GAIN` (Part 1 still uses legacy `OSC_TYPE` + `LAYER_*`).
- **UI:** `PartSelectorBar` (1–8 + MULTI), `MultiMixerPanel` (8 strips + master + tier).
- **Docs:** [`features/OMNISPHERE_ARCHITECTURE.md`](features/OMNISPHERE_ARCHITECTURE.md) updated.

## [2026-06-27] Omnisphere Multi — Phase 0 architecture scaffold

- **Design:** [`features/OMNISPHERE_ARCHITECTURE.md`](features/OMNISPHERE_ARCHITECTURE.md) — 8 Parts × 4 layers (A–D), phased delivery.
- **DSP:** `Source/DSP/Multi/` — `MultiPatchState`, `SynthPartState`, `PartLayerState`, `LegacyPartBridge` (Part 1 ← current matrix).
- **Processor:** `multiPatch` synced each block; not yet driving audio (Parts 2–8 silent).
- **Tests:** `MultiPatch_test.cpp` — part mute/solo rules.

## [2026-06-27] DSP tests + LFO routing model

- **Catch2 unit tests:** `Softysn1_DSP_Tests` target (`SOFTYSN1_BUILD_TESTS=ON`) — mod matrix curves, LFO quick-assign stacking, `SaturateSample`, PolyBLEP, ADSR sustain.
- **`Source/DSP/SaturateSample.h`:** Shared tanh saturator (voice drive + tests).
- **Dual LFO routing resolved:** Quick Dest (`LFO*_DEST`) and MOD matrix routes **sum additively**; documented in `LfoDestRouting.h`, matrix hint label, LFO row tooltips; APVTS renamed to "Quick Dest".

## [2026-06-27] Mod matrix — 10 new destinations

- **`ModDest` expanded:** FM Index, FM Ratio, FM6 Index, FM6 Op2 Ratio, Granular Pos, WT Warp, Ring Mod, Layer B/C/D Level (20 routable destinations total; preset dest indices 0–10 unchanged).
- **DSP wiring:** `OscillatorModule` (FM, warp, ring), `SynthVoice` (granular scan), `LayerStack` (layer mixer levels).

## [2026-06-27] P0 — tail length, Parameters wiring, voice FX CPU, RT scratch

- **`getTailLengthSeconds()`:** Returns `8.0` so hosts keep rendering reverb/delay tails after note-off.
- **`Parameters.h`:** Expanded to 209 APVTS IDs + `Params::load()` helper; `PluginProcessor::processBlock` and `createParameterLayout` use constants (no raw string literals in RT path).
- **`Params::load()`:** Debug `jassertfalse` when an ID is missing (zero cost in release).
- **`SynthVoice`:** Removed per-voice `EffectChain` (`voiceRack`); ladder drive uses inline `SaturationModule::saturateSample`.
- **`SynthVoice`:** Pre-allocated `voiceScratch` buffer in `prepareToPlay` (eliminates `setSize` on audio thread).

## [2026-06-27] UI/UX — knob sizing + compact LFO/MSEG modulation panel

- **`BioSynthUI.h`:** New knob size constants `kKnobS=48`, `kKnobM=64`, `kKnobL=88` and `layoutKnobsInRowSized()` helper that caps knob width to a target size and centres the group (fixes squished FX rack knobs and over-wide ADSR knobs).
- **Knob standardization:** 10 panels routed through the sized helper — `FXRackPanel`/`SampleMapPanel` use `kKnobS`; `OscColumnHub`/`ModulationPanel`/`PerformPanel`/`SettingsPanel`/`OscOrganismsPanel` use `kKnobM`; `SynthesisCorePanel` ADSR/master use `kKnobL`. `LayersPanel` mixer knobs capped to `kKnobM` width.
- **ModulationPanel rewrite:** Replaced the 6-tab mutually-exclusive system with a **stacked compact list** — all 4 LFOs + 2 MSEGs are visible simultaneously. Each row has a mini shape preview, combos, 3 knobs, and a `DRAW`/`EDIT` button.
- **LFO custom-draw modal:** Clicking `DRAW` opens a resizable `LfoDrawEditor` dialog (`juce::DialogWindow`) with a full-size `LfoCustomDrawView` for 16-point custom waveform editing. Button is only enabled when the LFO shape is set to **Custom** (preset shapes don't need editing).
- **MSEG editor modal:** Each MSEG row has an `EDIT` button that opens a `MsegDrawEditor` dialog with the large graph + per-segment level/time/curve knobs.
- **Reuses existing components:** `BioWaveform::LfoCustomDrawView`, `MsegGraphEditor`, all APVTS bindings (`LFO*_SHAPE/RATE/W*`, `MSEG*_S*_LVL/TIME/CURVE`) — no DSP changes.

## [2026-06-27] Documentation — synthesizer reference + technical review

- **Added [`docs/BC_Softsyn_Synthesizer_Reference (1).md`](docs/BC_Softsyn_Synthesizer_Reference%20(1).md):** 17-section guide (synth types, building blocks, DSP, software architecture, UI/UX theory, genres).
- **Added [`docs/issues.md`](docs/issues.md):** Independent code review — P0 bugs (tail length, APVTS literals, per-voice FX CPU, RT alloc) and architecture notes.
- **ROADMAP:** New P0 table, expanded P2/P3 from review; documentation map + parity snapshot vs reference guide.

## [2026-06-27] Sprint 6 — Step mod matrix source

- **StepModEngine:** Host-synced 8/16/32-step modulator (1/4–1/32 rate, swing); outputs 0..1 to `VoiceParameterMatrix`.
- **Mod matrix:** New source `Step Mod` (`ModSource::StepMod` = 26); routable to WT position, filter, etc.
- **UI:** `StepModPanel` on MOD tab (above MATRIX) — vertical step levels + playhead highlight.
- **Params:** `STEP_MOD_ON`, `STEP_MOD_STEPS`, `STEP_MOD_DIV`, `STEP_MOD_SWING`, `STEP_MOD_LVL_0..15`.

## [2026-06-27] Standards — strict rules & greenfield template

- **PROJECT_STANDARDS §7–§9:** Must/must-not rules, migration backlog, greenfield day-1 checklist.
- **docs/GREENFIELD_CHECKLIST.md:** Copy-paste scaffold for the next JUCE plugin project.

## [2026-06-27] Architecture — Core / DSP / UI decoupling

- **Moved wrapper to `Source/Core/`:** `PluginProcessor`, `PluginEditor`, `PresetManager`; added `Parameters.h` (APVTS ID registry).
- **Modular FX:** Six effect modules under `Source/DSP/EffectModules/{Compressor,Delay,Reverb,Chorus,EQ,Distortion}/`.
- **DSP state:** `VoiceParameterMatrix` → `Source/DSP/State/`.
- **UI:** `BioSynthLookAndFeel` → `Source/UI/LookAndFeel/`; scaffold `Tests/DSP_Tests/`.
- **CMake:** Updated source paths; VST3 build verified. Legacy shim headers retained at old paths.

## [2026-06-27] Project standards — folder structure

- **PROJECT_STANDARDS §1.1:** Best-practice directory tree (generic + Softysn1-specific) and “where to put new work” table.
- **docs/FOLDER_STRUCTURE.md:** Quick path reference linking to SOP.

## [2026-06-27] Sprint 6 — presets & project standards

- **Factory presets:** Refined 26 classics; added 26 new patches (FM6, comb/formant, WT warp, layers, arp). **52+** presets in `Presets/Factory/`; generator `tools/build_factory_presets.py`.
- **PATCHES categories:** Added KEYS and SEQ filter nodes in patch browser.
- **PROJECT_STANDARDS:** Master `ROADMAP.md` + `CHANGELOG.md` at repo root; satellite docs redirect here.
- **FX rack:** Basic per-slot waveform preview stub (`EffectSlotUI::drawEffectPreview`).

## [2026-06-27] Sprint 5 P2 — engines & modulation depth

- **6-operator FM:** `FM6` osc type, 8 algorithms, `FM6_*` params.
- **Filters:** Comb + Formant types; dual filter serial/parallel routing.
- **Wavetable:** Warp modes (Bend, Mirror, Quantize, Spectral) via `WAVETABLE_WARP_*`.
- **Mod matrix:** 8 → 16 slots; curve shaping (Linear / Expo / Log / S-Curve / Invert).
- **Layer mixer:** Per-layer pan, mute, solo (A/B/C/D).
- **MOD UI:** Tabbed LFO 1–4 + MSEG 1–2; draggable ADSR (Core) and MSEG graphs; LFO custom draw (16 points).
- **MSEG2:** Second engine + mod source.
- **MPE v1:** Settings toggle + CC74 timbre.

## [2026-06-27] Sprint 5 P1 — sampling, drums, presets

- **Multisample:** 4 independent zone buffers + velocity Lo/Hi per zone.
- **Drum kit:** `DrumKitEngine` + step sequencer + KIT toggle.
- **Preset bar:** Header ◀ ▶ name SAVE ☆.
- **Factory bank:** Expanded to 26 presets with audible mod routing examples.

## [2026-06-26] Sprint 4 — flagship architecture

- **Layers:** Multi-layer B/C/D stack.
- **Performance:** Arpeggiator, chorder, scale quantize, on-screen keyboard.
- **Physical modeling:** Karplus + Modal osc types.
- **Scripting:** Lua console (SYSTEM).
- **UI:** GUI workspace presets (7), UI scale 75–200%, AU export.
- **WT editor:** Draw, FFT smooth, resample from sample.

## [2026-06-25] Sprint 3 — Serum-inspired layout

- **ENGINE:** `OscColumnHub` columnar OSC UI + wavetable editor panel.
- **PATCHES:** Triple-pane browser, categories, tags, favorites, TAG+ / CAT→ batch tools.
- **MIXER:** Vertical bus strips, modular FX rack (4 lanes).

## [2026-06-24] Sprint 2 — sound & presets

- **LFO:** Waveform previews; LFO3/4 destination routing.
- **Presets:** Factory install pipeline + category metadata.

## [2026-06-23] Sprint 1 — UX polish

- **Visualizer:** Output scope replaces CPU bar graph; CPU % in header.
- **OSC:** Larger WT position knob; unison detune sensitivity fix.
- **Paths:** Remember last folder for WT/sample dialogs (`FileDialogPaths`).
- **Theme:** Font sizes + tighter knob layout.

## [2026-06-20] Foundation — Bio-Synth v2.0 shell

- **UI:** Bottom-nav hubs (ENGINE / MOD / MIXER / PATCHES / SYSTEM).
- **DSP:** 16-voice poly; VA, WT, granular, sampler; mod matrix; 4 LFOs; master FX.
- **Build:** JUCE + CMake — VST3, AU, Standalone (macOS).

---

*Append new entries at the top under a dated sprint heading. Keep bullets to one line each.*
