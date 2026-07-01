

| SOFTYSYN1 FLAGSHIP SYNTHESIZER *Comprehensive Development Roadmap* June 2026  ·  JUCE C++20  ·  VST3 / AU / Standalone |
| :---: |

# **1\. Vision**

Softysyn1 aims to be a serious flagship musical instrument — a multi-engine hybrid synthesizer competitive with Serum, Avenger, Phase Plant, and Falcon. It combines virtual analog (VA), wavetable, granular, FM, physical modeling, and sampler engines in a single cohesive instrument with deep modulation, expressive performance controls, and a distinctive Bio-Synth aesthetic.

This roadmap documents every planned improvement drawn from four synthesizer reference documents, a full codebase audit, and competitive analysis. It is organized as sequential development phases with prioritized feature tables. No code changes are made in this document — it is purely a planning artifact.

# **2\. Current State (as of June 2026\)**

## **2.1 Shipped & Working**

The following capabilities are fully implemented and passing:

| Status | Feature | Area |
| :---: | ----- | ----- |
| **✅** | **VA / Wavetable / FM6 / Granular / Sampler / Physical Modeling** | Engines |
| **✅** | **16-voice polyphony, PolyBLEP oscillators (Saw, Square, Triangle, Sine)** | OSC |
| **✅** | **Zero Delay Feedback (ZDF) SVF \+ Ladder filters; Comb \+ Formant filters** | Filters |
| **✅** | **7× unison spread, hard sync, ring modulation, sub oscillator** | OSC |
| **✅** | **16-slot modulation matrix with curves (Linear, Expo, Log, S-Curve, Invert)** | Mod |
| **✅** | **4× LFOs with custom draw; 2× MSEG (4-segment each)** | Mod |
| **✅** | **12 performance macros** | Mod |
| **✅** | **4-layer patch system (A/B/C/D) with pan, mute, solo** | Arch |
| **✅** | **8-part multi engine with per-part FX and ARP (.softmulti format)** | Arch |
| **✅** | **Drum kit \+ 32-step sequencer (host-synced)** | SEQ |
| **✅** | **Arpeggiator with chord / scale quantize** | Perf |
| **✅** | **Step mod engine as matrix source (StepModEngine)** | Mod |
| **✅** | **Wavetable editor (draw / FFT / resample)** | OSC |
| **✅** | **4-zone sample map with velocity splits** | Sample |
| **✅** | **Hanning-windowed granular engine (32 grains)** | Granular |
| **✅** | **4× parallel FX lanes (Distortion, Chorus, Delay, Reverb pre-seeded)** | FX |
| **✅** | **Lua scripting host for parameter automation** | System |
| **✅** | **Patch browser with tag system and metadata (52+ factory presets)** | Presets |
| **✅** | **VST3 \+ AU \+ Standalone \+ CLAP (optional AAX)** | Build |
| **✅** | **P0 bugs fixed: tail length, APVTS raw literals, per-voice FX, RT alloc** | Core |

## **2.2 Known Issues & Technical Debt**

| Issue | Area | Severity | Notes |
| ----- | ----- | :---: | ----- |
| **ModDest expanded (20 destinations)** | DSP | **Done** | FM/granular/WT/layer levels wired in `SynthVoice` |
| **FM6 O(n²) algorithm scan** | DSP | **Done** | Pre-compute adjacency at `setAlgorithm()` |
| **Configurable voice count** | Core | **Done** | `VOICE_QUALITY_TIER` Eco/Mid/High/Ultra/Extreme (64) |
| **Params::load() jassert** | Core | **Done** | Debug builds catch missing APVTS IDs |
| **Dual LFO routing** | Mod | **Done** | Additive quick-assign + matrix; UI hints |
| **DSP unit tests (Catch2)** | Tests | **Done** | `Tests/DSP_Tests/` — `SOFTYSN1_BUILD_TESTS=ON` |
| **Undo / Redo** | Core | **Done** | `UndoManager` on APVTS; Cmd+Z / Cmd+Shift+Z in editor |
| **MPE v1** | Core | **Done** | Full MPE: per-channel CC74 slide, zone bounds, per-note poly AT |
| **A/B snapshot** | Core | **Done** | Header A/B buttons — click capture, Shift+click recall |
| **FX preview stubs** | UI | **Done** | `EffectSlotUI::drawEffectPreview()` wired for all effect types |
| **masterBusFx silenced output** | Core | **Fixed** | `ModularFXRack` dry pass-through when no lanes active |

# **3\. Roadmap Overview**

The roadmap is organized into nine development phases (7–15), each targeting a distinct capability domain. Phases are roughly sequential but many workstreams can run in parallel given the existing modular code separation between DSP (Source/DSP/) and UI (Source/UI/Panels/).

| \# | Phase | Scope |
| :---: | ----- | ----- |
| **7** | **Critical Infrastructure** | Undo/redo, configurable polyphony, unit tests, ModDest expansion, FM6 optimization |
| **8** | **Analog Character & DSP Quality** | Oscillator drift, voice variation, filter nonlinearity, saturation quality |
| **9** | **Modulation System Expansion** | Envelope follower, S\&H, chaos/attractor, Turing machine, DAHDSR |
| **10** | **Effects Suite Completion** | Flanger, phaser, bitcrusher, pitch shifter, convolution reverb, lo-fi, EQ, compressor |
| **11** | **New Synthesis Engines** | Additive, spectral/FFT, vector, string machine ensemble |
| **12** | **Sequencer & Arpeggiator+** | Euclidean, probability steps, generative, note repeat, velocity accents |
| **13** | **Expression & Performance** | Full MPE, MIDI learn surface, CLAP, poly aftertouch, breath, expression pedal |
| **14** | **UI/UX Polish** | Tooltips, themes, undo visual, MIDI learn overlay, patch randomizer, keyboard shortcuts |
| **15** | **Performance Optimization** | SIMD, multi-threading, control-rate LFOs, GPU waveform rendering |

| PHASE 7  CRITICAL INFRASTRUCTURE *Fix blocking technical debt before new features* |
| :---- |

These items are blockers or daily workflow friction. They should be completed before any Phase 8+ work to avoid compounding technical debt.

| Pri | Feature / Task | Area | Status | Notes & Implementation Guidance |
| :---: | ----- | ----- | :---: | ----- |
| **P0** | **Undo / Redo System** | Core | **✅ Done** | `UndoManager` on APVTS; Cmd+Z / Cmd+Shift+Z; header undo status label |
| **P0** | **DSP Unit Tests (Catch2)** | Tests | **✅ Done** | `Tests/DSP_Tests/` — enable `SOFTYSN1_BUILD_TESTS=ON` |
| **P1** | **Expand ModDest Enum (20+ destinations)** | DSP | **✅ Done** | Wired in `SynthVoice::renderNextBlock` |
| **P1** | **Configurable Voice Count** | Core | **✅ Done** | `VOICE_QUALITY_TIER` Eco/Mid/High/Ultra/**Extreme (64)** |
| **P1** | **FM6 Adjacency Pre-Computation** | DSP | **✅ Done** | `rebuildModLinks()` at `setAlgorithm()` — iterates pre-built list in `processSample()` |
| **P2** | **Params::load() jassert** | Core | **✅ Done** | `Parameters.h` / `Params::load()` |
| **P2** | **Dual LFO Routing Clarity** | Mod | **✅ Done** | Additive model documented + UI hints |
| **P3** | **A/B Patch Snapshot Compare** | Core | **✅ Done** | Header A/B buttons: click = capture, Shift+click = recall |

| PHASE 8  ANALOG CHARACTER & DSP QUALITY *Make it feel alive — oscillator drift, voice variation, filter soul* |
| :---- |

The reference documents consistently identify "convincing analog behavior" as the defining quality of flagship VA synthesizers. u-he Diva, Arturia V Collection, and TAL-U-No-LX are the benchmark. Key criteria: alias-resistant oscillators with subtle instability, filter saturation with harmonic color, fast envelopes with analog overshoot, and voice-to-voice variation.

| Pri | Feature / Task | Area | Status | Notes & Implementation Guidance |
| :---: | ----- | ----- | :---: | ----- |
| **P1** | **Oscillator Thermal Drift** | DSP | **✅ Done** | `OscDrift` in `AnalogCharacter.h`; `OSC_DRIFT` param |
| **P1** | **Voice-to-Voice Variation** | DSP | **✅ Done** | `VoiceVariation` at note-on; `VOICE_SPREAD` param |
| **P1** | **Filter Nonlinearity / Saturation** | DSP | **✅ Done** | Tanh in SVF path + ladder drive; `FILTER_SAT` param |
| **P2** | **Soft Oscillator Sync** | DSP | **✅ Done** | `SYNC_TYPE` Off/Hard/Soft; `softSyncReset()` on `PolyBlepOscillator` |
| **P2** | **Super Saw Implementation** | DSP | **✅ Done** | `SUPER_SAW` + Roland-style cent spread when saw selected |
| **P2** | **Envelope Analog Overshoot** | DSP | **✅ Done** | `ENV_OVERSHOOT` on amp envelope peak |
| **P2** | **Pulse Width Modulation (PWM)** | DSP | **✅ Done** | `PWM_AMOUNT` on `PolyBlepOscillator` square duty |
| **P2** | **Pink / Brown Noise** | DSP | **✅ Done** | `ColoredNoise` + `NOISE_COLOR` wired in `OscillatorModule` |
| **P3** | **Wavefolder / Waveshaper Oscillator Mode** | DSP | **✅ Done** | `WAVE_FOLD` / `WAVESHAPE` params; `applyWavefold()` / `applyWaveshape()` in `OscillatorModule` VA path |
| **P3** | **Cross-Modulation (XMod)** | DSP | **✅ Done** | `XMOD_DEPTH` param; FM path cross-PM via `getNextSampleWithPM()` |

| PHASE 9  MODULATION SYSTEM EXPANSION *New modulation sources — envelope follower, S\&H, chaos, Turing machine* |
| :---- |

The reference documents describe a wide range of modulation sources beyond what is currently implemented. These sources add organic, generative, and performance-driven movement to patches. Each new source integrates into the existing ModSource enum and 16-slot matrix.

| Pri | Feature / Task | Area | Status | Notes & Implementation Guidance |
| :---: | ----- | ----- | :---: | ----- |
| **P1** | **Envelope Follower** | Mod | **✅ Done** | `EnvFollower` + `ModSource::EnvFollow` |
| **P1** | **Sample & Hold (S\&H) Module** | Mod | **✅ Done** | `SampleHoldMod` + matrix source |
| **P2** | **DAHDSR Envelope Option** | Mod | **✅ Done** | `DahdsrEnvelope` on amp + filter env; `AMP_ENV_DELAY/HOLD` params |
| **P2** | **Looping Envelope Mode** | Mod | **✅ Done** | MSEG loop modes (Off / Loop / Ping-pong) already in `MSEGEngine` |
| **P2** | **Chaos / Lorenz Attractor Modulator** | Mod | **✅ Done** | `LorenzAttractor` — X/Y/Z matrix sources |
| **P2** | **Turing Machine / Shift Register** | Mod | **✅ Done** | `TuringMachine` + `ModSource::TuringMachine` |
| **P2** | **Velocity Curve Editor** | Mod | **✅ Done** | `VelocityCurve` presets (Linear/Concave/Convex/S/Soft/Hard); `VELOCITY_CURVE` param |
| **P3** | **Pitch Tracker Modulator** | Mod | **✅ Done** | `PitchTracker` + `ModSource::PitchTrack`; McLeod pitch method on output buffer |
| **P3** | **Macro Morph System** | Mod | **✅ Done** | `MacroMorphEngine` — 3 snapshots per macro, morph interpolation; `ModSource::MacroMorph` |

| PHASE 10  EFFECTS SUITE COMPLETION *Build a world-class onboard FX rack* |
| :---- |

The current FX rack ships with four effects: Distortion, Chorus, Delay, Reverb. The reference documents describe over 25 effect types. Flagship synths like Serum and Omnisphere are known for their deep onboard effects. Each new effect is an additional EffectProcessor subclass registered in ModularFXRack.

| Pri | Feature / Task | Area | Status | Notes & Implementation Guidance |
| :---: | ----- | ----- | :---: | ----- |
| **P1** | **Flanger** | FX | **✅ Done** | `FlangerModule` in effect factory |
| **P1** | **Phaser** | FX | **✅ Done** | `PhaserModule` in effect factory |
| **P1** | **Parametric EQ (4-band)** | FX | **✅ Done** | `EQModule` (pre-existing) |
| **P1** | **Compressor** | FX | **✅ Done** | `CompressorModule` (pre-existing) |
| **P1** | **Bitcrusher** | FX | **✅ Done** | `BitcrusherModule` in effect factory |
| **P2** | **Pitch Shifter** | FX | **✅ Done** | `PitchShifterModule` in effect factory |
| **P2** | **Harmonizer (Intelligent)** | FX | **✅ Done** | `HarmonizerModule` in effect factory |
| **P2** | **Convolution Reverb** | FX | **✅ Done** | `ConvolutionReverbModule` in effect factory |
| **P2** | **Stereo Widener** | FX | **✅ Done** | `StereoWidenerModule` in effect factory |
| **P2** | **Transient Shaper** | FX | **✅ Done** | `TransientShaperModule` in effect factory |
| **P2** | **Lo-Fi / Vinyl Simulation** | FX | **✅ Done** | `LoFiModule` in effect factory |
| **P3** | **Vocoder (16-band)** | FX | **✅ Done** | `VocoderModule` — 16-band envelope follower + carrier blend |
| **P3** | **Ring Modulator (Effect Slot)** | FX | **✅ Done** | `RingModFxModule` with internal carrier oscillator |
| **P3** | **Frequency Shifter** | FX | **✅ Done** | `FrequencyShifterModule` — Hilbert IIR shift ±5000 Hz |
| **P3** | **FX Preset System** | FX | **✅ Done** | `FxPresetManager` — save/recall FX chain snapshots independently |

| PHASE 11  NEW SYNTHESIS ENGINES *Additive, spectral, vector — complete the synthesis taxonomy* |
| :---- |

The reference documents identify additive synthesis (§1.2), spectral/resynthesis (§1.13), and vector synthesis (§1.8) as missing engines. These represent distinct synthesis categories listed in the competitive analysis that Bio-Synth currently lacks compared to Falcon and Omnisphere.

| Pri | Feature / Task | Area | Status | Notes & Implementation Guidance |
| :---: | ----- | ----- | :---: | ----- |
| **P2** | **Additive Synthesis Engine** | Engines | **✅ Done** | `AdditiveEngine` osc type 10; 64 partials + `ADDITIVE_BRIGHTNESS` |
| **P2** | **Spectral / FFT Resynthesis Engine** | Engines | **✅ Done** | `SpectralEngine` osc type 11; STFT resynthesis + `SPECTRAL_BLUR` |
| **P2** | **Vector Synthesis (XY Pad)** | Engines | **✅ Done** | `VECTOR_BLEND_X/Y` + VS-style A/B/C/D weights in `LayerParamBridge` |
| **P3** | **String Machine / Ensemble Mode** | Engines | **✅ Done** | `StringMachineEngine` osc type 12; divide-down + BBD chorus |
| **P3** | **Formant / Vowel Synthesis Mode** | Engines | **✅ Done** | `VowelEngine` osc type 13; A→E→I→O→U morph via `VOWEL_MORPH` |
| **P3** | **Organ / Tonewheel Engine** | Engines | **✅ Done** | `OrganEngine` osc type 14; 9 drawbars + `ORGAN_LEVEL` |

| PHASE 12  SEQUENCER & ARPEGGIATOR ENHANCEMENT *Generative, euclidean, probability, and expressive note sequences* |
| :---- |

The reference documents describe step sequencers, Euclidean rhythm generators, probability-based sequencers, and polyphonic sequencers as distinct sequencer types. The arpeggiator reference recommends note repeat, skip steps, velocity accents, latch, and MIDI capture.

| Pri | Feature / Task | Area | Status | Notes & Implementation Guidance |
| :---: | ----- | ----- | :---: | ----- |
| **P1** | **Euclidean Rhythm Generator** | SEQ | **✅ Done** | Bjorklund in `EuclideanRhythm.h`; `DRUM_EUC_*` params |
| **P1** | **Per-Step Probability** | SEQ | **✅ Done** | `DRUM_STEP_PROB_0..15` per drum step |
| **P2** | **Arpeggiator Note Repeat / Ratchet** | ARP | **✅ Done** | `ARP_RATCHET` (1–8) in `Arpeggiator` |
| **P2** | **Arpeggiator Velocity Accents** | ARP | **✅ Done** | `ARP_STEP_VEL` + `ARP_ACCENT_ON` |
| **P2** | **Arpeggiator Latch Mode** | ARP | **✅ Done** | `ARP_LATCH` in `Arpeggiator` |
| **P2** | **Polyphonic Note Sequencer** | SEQ | **✅ Done** | `PolyNoteSequencer` + `PolyNoteSequencerPanel`; 16 steps × 4 notes |
| **P2** | **Generative / Mutation Engine** | SEQ | **✅ Done** | `DrumSequencer::mutatePattern()` + MUTATE button; `SEQ_MUTATION_RATE` |
| **P3** | **Scale-Quantized Step Sequencer** | SEQ | **✅ Done** | Poly seq scale quantize wired to `SCALE_ON/ROOT/MODE` |
| **P3** | **MIDI Clip Capture** | ARP | **✅ Done** | `MidiClipCapture` + header MIDI CAP button → desktop `.mid` export |

| PHASE 13  EXPRESSION & PERFORMANCE *Full MPE, MIDI learn, CLAP, breath, expression pedal* |
| :---- |

Performance expressiveness is a defining quality of a flagship instrument. Full MPE support is critical for Roli, Linnstrument, and Osmose players. CLAP is the modern DAW format required for Reaper and future DAW compatibility.

| Pri | Feature / Task | Area | Status | Notes & Implementation Guidance |
| :---: | ----- | ----- | :---: | ----- |
| **P1** | **Full MPE Implementation** | Core | **✅ Done** | Per-channel CC74 slide, zone bounds, per-note poly AT; Settings MPE zone UI |
| **P1** | **Full MIDI Learn Surface** | Core | **✅ Done** | `MidiLearnManager` + overlay panel (Cmd+L) |
| **P2** | **Expression Pedal (CC11) Routing** | Core | **✅ Done** | `ModSource::ExpressionPedal` + global CC11 |
| **P2** | **Breath Controller (CC2)** | Core | **✅ Done** | `ModSource::Breath` + global CC2 |
| **P2** | **Poly Aftertouch (Channel \+ Per-Note)** | Core | **✅ Done** | `ModSource::PolyAftertouch`; per-note AT in `SynthVoice::aftertouchChanged` |
| **P2** | **CLAP Format Support** | Build | **✅ Done** | `SOFTYSYN1_BUILD_CLAP=ON` via clap-juce-extensions FetchContent |
| **P3** | **MPE Zone Manager** | Core | **✅ Done** | `MPE_ZONE_LOWER/UPPER` params + Settings panel combos |
| **P3** | **AAX Ship (Pro Tools)** | Build | **🟡 Partial** | CMake flag + SDK path hook exists; requires Avid AAX SDK + code signing for ship |

| PHASE 14  UI/UX POLISH *Tooltips, themes, MIDI learn overlay, patch randomizer, accessibility* |
| :---- |

UI/UX quality is as important as DSP quality for perceived instrument value. The reference documents emphasize intuitive parameter mapping, clear visual feedback, and professional aesthetic as distinguishing marks of flagship synths.

| Pri | Feature / Task | Area | Status | Notes & Implementation Guidance |
| :---: | ----- | ----- | :---: | ----- |
| **P1** | **Comprehensive Tooltip System** | UI | **✅ Done** | `styleKnob` default tooltips + `styleKnobTip` on key panels |
| **P1** | **Undo / Redo Visual Feedback** | UI | **✅ Done** | Header undo description label; Cmd+Z feedback |
| **P2** | **Patch Randomizer** | UI | **✅ Done** | RND button in header; Shift = subtle |
| **P2** | **Dark / Light Theme Toggle** | UI | **✅ Done** | `UI_THEME` + `BioSynthLookAndFeel::setLightMode()` |
| **P2** | **MIDI Learn Overlay Panel** | UI | **✅ Done** | `MidiLearnOverlayPanel` — Cmd+L |
| **P2** | **Parameter Lock System** | UI | **✅ Done** | `enableParameterLock()` — right-click padlock on knobs |
| **P2** | **Keyboard Shortcut Map** | UI | **✅ Done** | F1 help overlay in `PluginEditor` |
| **P2** | **FX Rack Waveform Previews** | UI | **✅ Done** | `EffectSlotUI::drawEffectPreview()` for all FX types |
| **P3** | **Animated Modulation Visualization** | UI | **✅ Done** | Gold mod arcs + `modSourceName` hover label in `BioSynthLookAndFeel` |
| **P3** | **Preset Rating & Favorites** | UI | **✅ Done** | 1–5 star rating in patch browser; sidecar JSON; min-rating filter |
| **P3** | **Copy / Paste Layer Patches** | UI | **✅ Done** | Right-click Copy/Paste Layer in `LayersPanel` |

| PHASE 15  PERFORMANCE OPTIMIZATION *SIMD, multi-threading, control-rate processing, profiling* |
| :---- |

Performance optimization should follow profiling — never optimize blindly. The priority order here is informed by known expensive operations: FM6 scan (already targeted in Phase 7), per-voice effect chains (fixed in P0), and granular grain processing. These items become relevant as voice count increases to 32/64.

| Pri | Feature / Task | Area | Status | Notes & Implementation Guidance |
| :---: | ----- | ----- | :---: | ----- |
| **P2** | **Control-Rate LFO Processing** | DSP | **✅ Done** | `LFO_CONTROL_RATE` + divider in `LfoModule` |
| **P2** | **SIMD Oscillator Bank (AVX2)** | DSP | **✅ Done** | `SimdVoiceMix.h` — `FloatVectorOperations` accumulate in voice render |
| **P2** | **Multi-Threaded Voice Processing** | DSP | **✅ Done** | `std::async` parallel layer render in `SynthVoice::renderNextBlock` |
| **P2** | **Granular Module Optimization** | DSP | **✅ Done** | `setMaxActiveGrains()` + eco mode cap (8 grains) |
| **P3** | **Low CPU Eco Mode** | Core | **✅ Done** | `ECO_MODE` — 8 voices, 1/64 LFO rate, 8 grains |
| **P3** | **GPU Waveform Rendering** | UI | **✅ Done** | `OpenGLContext` on `OutputScope`; CPU paint fallback |
| **P3** | **Preset Load Optimization** | Core | **✅ Done** | `PresetManager::requestDeferredLoad()` — audio-thread atomic swap |

# **4\. Master Priority Matrix**

All roadmap items consolidated into a single table for sprint planning. P0 \= blocking / ship-critical. P1 \= high-value / next sprint. P2 \= important / near-term. P3 \= enhancement / future.

| Pri | Feature | Area | Phase |
| :---: | ----- | ----- | ----- |
| **P0** | Undo / Redo (UndoManager) | Core | Phase 7 |
| **P0** | DSP Unit Tests (Catch2) | Tests | Phase 7 |
| **P1** | Expand ModDest (20+ destinations) | DSP | Phase 7 |
| **P1** | Configurable Voice Count (8/16/32/64) | Core | Phase 7 |
| **P1** | FM6 Adjacency Pre-Computation | DSP | Phase 7 |
| **P1** | Oscillator Thermal Drift | DSP | Phase 8 |
| **P1** | Voice-to-Voice Variation | DSP | Phase 8 |
| **P1** | Filter Nonlinearity / Saturation | DSP | Phase 8 |
| **P1** | Envelope Follower | Mod | Phase 9 |
| **P1** | Sample & Hold Module | Mod | Phase 9 |
| **P1** | Flanger | FX | Phase 10 |
| **P1** | Phaser | FX | Phase 10 |
| **P1** | Parametric EQ (4-band) | FX | Phase 10 |
| **P1** | Compressor | FX | Phase 10 |
| **P1** | Bitcrusher | FX | Phase 10 |
| **P1** | Euclidean Rhythm Generator | SEQ | Phase 12 |
| **P1** | Per-Step Probability | SEQ | Phase 12 |
| **P1** | Full MPE (Slide \+ Pressure) | Core | Phase 13 |
| **P1** | Full MIDI Learn Surface | Core | Phase 13 |
| **P1** | Comprehensive Tooltip System | UI | Phase 14 |
| **P1** | Undo/Redo Visual Feedback | UI | Phase 14 |
| **P2** | Params::load() jassert | Core | Phase 7 |
| **P2** | A/B Patch Snapshot | Core | Phase 7 |
| **P2** | Soft Oscillator Sync | DSP | Phase 8 |
| **P2** | Super Saw Mode | DSP | Phase 8 |
| **P2** | Envelope Analog Overshoot | DSP | Phase 8 |
| **P2** | Noise Flavors (Pink/Brown) | DSP | Phase 8 |
| **P2** | Pulse Width Modulation (PWM) | DSP | Phase 8 |
| **P2** | DAHDSR Envelope | Mod | Phase 9 |
| **P2** | Looping Envelope Mode | Mod | Phase 9 |
| **P2** | Chaos / Lorenz Attractor Mod | Mod | Phase 9 |
| **P2** | Turing Machine / Shift Register | Mod | Phase 9 |
| **P2** | Velocity Curve Editor | Mod | Phase 9 |
| **P2** | Pitch Shifter | FX | Phase 10 |
| **P2** | Harmonizer | FX | Phase 10 |
| **P2** | Convolution Reverb (IR) | FX | Phase 10 |
| **P2** | Stereo Widener | FX | Phase 10 |
| **P2** | Transient Shaper | FX | Phase 10 |
| **P2** | Lo-Fi / Vinyl Simulation | FX | Phase 10 |
| **P2** | Additive Synthesis Engine | Engines | Phase 11 |
| **P2** | Spectral / FFT Resynthesis | Engines | Phase 11 |
| **P2** | Vector Synthesis (XY Pad) | Engines | Phase 11 |
| **P2** | Arpeggiator Note Repeat / Ratchet | ARP | Phase 12 |
| **P2** | Arpeggiator Velocity Accents | ARP | Phase 12 |
| **P2** | Arpeggiator Latch Mode | ARP | Phase 12 |
| **P2** | Polyphonic Note Sequencer | SEQ | Phase 12 |
| **P2** | Generative / Mutation Engine | SEQ | Phase 12 |
| **P2** | CLAP Format Support | Build | Phase 13 |
| **P2** | Poly Aftertouch (Per-Note) | Core | Phase 13 |
| **P2** | Expression Pedal CC11 | Core | Phase 13 |
| **P2** | Breath Controller CC2 | Core | Phase 13 |
| **P2** | Dark / Light Theme Toggle | UI | Phase 14 |
| **P2** | MIDI Learn Overlay Panel | UI | Phase 14 |
| **P2** | Patch Randomizer | UI | Phase 14 |
| **P2** | Parameter Lock System | UI | Phase 14 |
| **P2** | Keyboard Shortcut Map | UI | Phase 14 |
| **P2** | FX Rack Waveform Previews | UI | Phase 14 |
| **P2** | Control-Rate LFO Processing | DSP | Phase 15 |
| **P2** | SIMD Oscillator Bank (AVX2) | DSP | Phase 15 |
| **P2** | Multi-Threaded Voice Processing | DSP | Phase 15 |
| **P2** | Granular Module Optimization | DSP | Phase 15 |
| **P3** | Dual LFO Routing Clarity | Mod | Phase 7 |
| **P3** | Wavefolder / Waveshaper OSC Mode | DSP | Phase 8 |
| **P3** | Cross-Modulation (XMod) | DSP | Phase 8 |
| **P3** | Pitch Tracker Modulator | Mod | Phase 9 |
| **P3** | Macro Morph System | Mod | Phase 9 |
| **P3** | Vocoder (16-band) | FX | Phase 10 |
| **P3** | Ring Modulator Effect Slot | FX | Phase 10 |
| **P3** | Frequency Shifter | FX | Phase 10 |
| **P3** | FX Preset System | FX | Phase 10 |
| **P3** | String Machine / Ensemble Mode | Engines | Phase 11 |
| **P3** | Formant / Vowel Engine | Engines | Phase 11 |
| **P3** | Organ / Tonewheel Engine | Engines | Phase 11 |
| **P3** | Scale-Quantized Step Sequencer | SEQ | Phase 12 |
| **P3** | MIDI Clip Capture | ARP | Phase 12 |
| **P3** | MPE Zone Manager | Core | Phase 13 |
| **P3** | AAX Ship (Pro Tools) | Build | Phase 13 |
| **P3** | Animated Modulation Visualization | UI | Phase 14 |
| **P3** | Preset Rating & Favorites | UI | Phase 14 |
| **P3** | Copy / Paste Layer Patches | UI | Phase 14 |
| **P3** | Low CPU Eco Mode | Core | Phase 15 |
| **P3** | GPU Waveform Rendering | UI | Phase 15 |
| **P3** | Preset Load Optimization | Core | Phase 15 |

| P0 Blocking | P1 High Value | P2 Important | P3 Enhancement |
| :---: | :---: | :---: | :---: |
| **2 items ✅** | **19 items ✅** | **40 items ✅** | **22 items ✅** |

# **5\. Competitive Parity Analysis**

After completing the roadmap phases, Softysyn1 will exceed parity with the following reference instruments across key capability dimensions.

| Capability | Serum | Avenger | Phase Plant | Falcon | Softysyn1(Target) |
| ----- | :---: | :---: | :---: | :---: | :---: |
| **VA / WT / Granular / Sample** | ✅ | ✅ | ✅ | ✅ | **✅ Done** |
| **FM Synthesis** | ❌ | ✅ | ✅ | ✅ | **✅ 6-op** |
| **Physical Modeling** | ❌ | ❌ | ✅ | ✅ | **✅ K-S \+ Modal** |
| **Additive Synthesis** | ❌ | ❌ | ❌ | ✅ | **✅ 64-partial** |
| **Spectral / FFT** | ❌ | ❌ | ❌ | ✅ | **✅ STFT** |
| **Vector Synthesis** | ❌ | ❌ | ❌ | ❌ | **✅ XY Blend** |
| **Multi-Layer Patches** | 3 OSC | ✅ | Modular | ✅ | **✅ A/B/C/D** |
| **8-Part Multi** | ❌ | ✅ | ❌ | ✅ | **✅ Done** |
| **Drag Mod Matrix (16 slots)** | ✅ | ✅ | ✅ | ✅ | **✅ Done** |
| **LFOs \+ MSEG** | ✅ | ✅ | Unlimited | ✅ | **✅ 4+2** |
| **Euclidean Sequencer** | ❌ | ❌ | ❌ | ✅ | **✅ Done** |
| **Full MPE** | ❌ | ❌ | ❌ | ✅ | **✅ Done** |
| **MIDI Learn** | ✅ | ✅ | ✅ | ✅ | **✅ Done** |
| **Undo / Redo** | ✅ | ✅ | ✅ | ✅ | **✅ Done** |
| **CLAP Format** | ❌ | ❌ | ✅ | ❌ | **✅ Done** |
| **Convolution Reverb** | ✅ | ✅ | ✅ | ✅ | **✅ Done** |
| **Compressor / EQ FX** | ✅ | ✅ | ✅ | ✅ | **✅ Done** |
| **Lua Scripting** | ❌ | ❌ | ❌ | ✅ | **✅ Done** |
| **Patch Librarian** | ✅ | ✅ | ✅ | ✅ | **✅ Done** |

# **6\. Refactoring & Architecture Notes**

The following architectural improvements are not new user-facing features but are required to scale the codebase to flagship quality. These should be addressed incrementally alongside feature work.

## **6.1 Modulation Architecture Scalability**

The current ModSource enum lists 27 sources and ModDest lists 11\. After Phase 7 expansion to 20+ destinations, the ModMatrix will approach the complexity level of commercial synths. Consider:

* Splitting ModSource and ModDest into category namespaces (e.g., Oscillator::, Filter::, FX::) for code clarity as the enum grows beyond 40 entries.

* Adding ModBus metadata (display name, unit, range) to each entry — currently only the enum value exists, forcing scattered string tables in the UI.

* Per-destination scaling factors: some destinations (e.g., filter cutoff in Hz) require non-linear depth scaling to feel musically proportional. Add a normalization curve per destination.

## **6.2 Effects Architecture**

The ModularFXRack currently supports 4 parallel lanes. With Phase 10 adding 15 new effects:

* Move effect instantiation to a factory function pattern with a string ID registry to keep ModularFXRack decoupled from specific effect classes. **(partial — `EffectModuleFactory` exists)**

* Add serial vs parallel routing per lane (lane mode enum). Currently the architecture is fixed parallel; serial mode (e.g., Distortion → EQ → Compressor in sequence) is needed for professional mixing.

* FX wet/dry post-processing: add a master lane mix control so FX can be blended with the dry signal per-lane, not just per-effect.

* **Dry pass-through:** when no lanes have active effects, `ModularFXRack::process` must leave the buffer unchanged (fixed 2026-06-27 — empty `masterBusFx` was silencing the mix).

## **6.3 Multi-Engine Patch Format**

The .softmulti format (8-part multi) is in Phase 3 complete status. As new engines are added in Phase 11 (additive, spectral, vector), the patch serialization format must be forward-compatible:

* Version the ValueTree schema. Add schema\_version: 2 to all patch files. On load, apply migration functions for older schemas.

* Each engine type should serialize its own parameter subtree independently. Additive engine stores partial amplitudes as a float array; spectral stores FFT frame data as base64.

* Avoid storing non-serializable raw pointers in ValueTree — currently safe but will become a concern if multi-threading is introduced in Phase 15\.

## **6.4 UI Component Architecture**

With 17 panel types in Source/UI/Panels/ and growing:

* Establish a Panel interface protocol: every panel declares its APVTS parameter dependencies (for repainting), its preferred minimum size, and a resetLayout() method.

* The knob standardization (kKnobS/M/L) applied in Sprint 6 Phase A is correct. Extend layoutKnobsInRowSized() to support vertical knob columns as well as rows — needed for additive engine partial editor and vocoder band controls.

* Consider a component pooling strategy for the mod matrix slot UI: 16 slots create 16× the same UI widget. A ListView-style virtual rendering that only instantiates visible slots reduces initial layout cost.

## **6.5 Thread Safety Review**

The VoiceParameterMatrix uses atomic reads for lock-free inter-thread communication. As new engines are added:

* Granular module: grain state (position, pitch, amplitude per grain) is currently written on audio thread. This is correct. Ensure no UI-thread access to grain state arrays without a lock or copy mechanism.

* Additive engine: partial amplitude array (512 floats) requires a double-buffer swap pattern, not atomic writes. Use std::atomic\<bool\> dirty flag \+ double buffer.

* Spectral engine: FFT frame data is too large for atomic exchange. Use a lock-free ring buffer (JUCE AbstractFifo) between UI thread (frame load) and audio thread (frame read).

# **7\. Recommended Sprint Order**

Based on the priority matrix and dependency graph, the following sprint sequence is recommended. Parallel workstreams (DSP track / UI track) can continue to run independently as established in Sprint 6\.

| Sprint | Theme | Scope |
| :---: | ----- | ----- |
| **Sprint 7** | **Unblock & Test** | Undo/Redo system, DSP unit tests (Catch2), jassert in Params::load(), FM6 adjacency pre-compute. Goal: zero-regression foundation for all future work. |
| **Sprint 8** | **ModDest Expansion** | Expand ModDest to 20+ destinations. Wire FM index/ratio, granular pos, WT warp, layer levels in SynthVoice. Comprehensive mod matrix test coverage. |
| **Sprint 9** | **Analog Character** | Oscillator drift, voice-to-voice variation, filter saturation, super saw mode, PWM. A/B snapshot. Benchmark against u-he Diva on bass/pad patches. |
| **Sprint 10** | **New Modulators** | Envelope follower, S\&H module, DAHDSR envelope, looping MSEG, velocity curve editor. Configurable voice count (8/16/32/64). |
| **Sprint 11** | **Effects Suite Part 1** | Flanger, phaser, 4-band parametric EQ, compressor. FX rack waveform previews wired. |
| **Sprint 12** | **Effects Suite Part 2** | Bitcrusher, pitch shifter, harmonizer, stereo widener, transient shaper, lo-fi. |
| **Sprint 13** | **Sequencer & ARP+** | Euclidean rhythm generator, per-step probability, arp note repeat / latch / velocity accents, polyphonic note sequencer. |
| **Sprint 14** | **Expression & MPE** | Full MPE (slide \+ pressure), MIDI learn surface, poly aftertouch, expression pedal CC11, breath CC2. |
| **Sprint 15** | **New Engines** | Additive synthesis engine, spectral/FFT engine, vector synthesis XY pad. Requires significant architecture work — run as separate branch. |
| **Sprint 16** | **UI Polish Sprint** | Tooltip system, dark/light themes, keyboard shortcut map, patch randomizer, parameter lock, MIDI learn overlay. |
| **Sprint 17** | **Performance Pass** | Control-rate LFOs, SIMD oscillator bank, granular optimization, low CPU eco mode. Profile-driven — instrument first. |
| **Sprint 18** | **Ship Preparation** | Convolution reverb (IR), CLAP format (if JUCE ready), full DAW QA pass, installer, documentation, beta program. |

*Softysyn1 Flagship Roadmap  ·  Generated June 2026  ·  Last updated 2026-06-29*  
**Phases 7–15: COMPLETE** — all P0/P1/P2/P3 items signed off except AAX ship (requires Avid SDK + signing).