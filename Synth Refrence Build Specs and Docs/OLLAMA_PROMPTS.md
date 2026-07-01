# SoftSyn_1 - Ollama Handoff Prompt Pack

This file is the bridge between your high-level planning model (e.g. Claude/GPT in Cursor)
and your local implementation model (Ollama in Cursor). The planning model authors and
updates these prompts; the local model does the implementation typing.

## How to use this file

1. In Cursor, switch the model to your local Ollama model (e.g. `qwen3-14b`, `code-llama`, `kimi-k2.5`).
2. Paste **Block 0 - Standing Context** once at the start of the local session. This primes the
   model with the project rules and file map so you don't repeat them.
3. Paste **Prompt 1** and let the local model implement it.
4. Run the build/verify loop. When it compiles clean and behaves, move on to **Prompt 2**, and so on.
5. Implement the prompts in order. Each prompt assumes the previous ones are already merged.

If a local model gets confused, re-paste Block 0, then re-paste only the single prompt you are on.
Keep one prompt = one feature = one build. Do not batch multiple prompts together.

---

# Block 0 - Standing Context (paste once per Ollama session)

```text
You are implementing features for an existing JUCE audio plugin called SoftSyn_1.
Follow these rules exactly. Do not redesign the architecture. Do not rename existing things.

PROJECT FACTS
- Framework: JUCE 8 (vendored in JUCE-master/). Language: C++20. Build system: CMake + Ninja.
- CMake target name: Softysn1. Product name: "_BC_softsyn1". Formats: VST3 + Standalone.
- It is a polyphonic synth: 16 instances of SynthVoice driven by juce::Synthesiser.
- All parameters live in a juce::AudioProcessorValueTreeState named "Parameters" (the variable apvts).
- State save/restore already works via XML in getStateInformation/setStateInformation. Any new
  parameter you add to the layout is automatically saved and restored. Never break this.

FILE MAP (only edit these unless told otherwise)
- Source/PluginProcessor.h / .cpp   -> AudioProcessor, APVTS, parameter layout, processBlock.
- Source/SynthVoice.h / .cpp        -> per-voice DSP (oscillators, filter, ADSR, gain).
- Source/WavetableOscillator.h/.cpp -> custom wavetable engine (example of a helper DSP class).
- Source/SynthSound.h               -> trivial SynthesiserSound.
- Source/PluginEditor.h / .cpp      -> GUI: sliders/combos + APVTS attachments.
- CMakeLists.txt                    -> the target_sources(Softysn1 ...) list. Register NEW .cpp/.h here.

THE 5-STEP PATTERN (every feature follows this)
1. Add parameter(s) in createParameterLayout() in PluginProcessor.cpp.
2. In processBlock(), read the parameter(s) with apvts.getRawParameterValue("ID")->load().
3. Pass the value(s) into each voice. The current path is voice->updateParameters(...). You may
   extend the updateParameters signature (update BOTH the declaration in SynthVoice.h and the
   definition in SynthVoice.cpp and the call site in PluginProcessor.cpp to match).
4. Implement the DSP in SynthVoice (or in a new helper class that SynthVoice owns).
5. Add a control + APVTS attachment in PluginEditor.h/.cpp so the parameter is usable in the UI.

HARD REAL-TIME RULES (audio thread safety - non-negotiable)
- NEVER allocate heap memory, lock a mutex, log, or read/write files inside processBlock() or
  renderNextBlock(). Do all allocation and setup in prepareToPlay().
- Use juce::dsp classes where possible. Keep juce::ScopedNoDenormals at the top of processBlock.
- Clamp/limit incoming parameter values defensively (juce::jlimit / jmax) like the existing code does.
- Per-voice scratch buffers must be sized in prepareToPlay, not in the render call.

OUTPUT RULES
- Only modify the files I list in the prompt. Do not touch JUCE-master/ or cmake-build-*/.
- When you change a function, output the COMPLETE function, not a fragment.
- Keep all existing parameter IDs and their string values stable (presets depend on them).
- Match the existing code style: JUCE brace style, juce:: prefixes, 4-space indent.

BUILD / VERIFY LOOP (run from the project root after each prompt)
- Configure (first time / after CMakeLists changes):  cmake --preset debug-ninja
- Build:                                              cmake --build cmake-build-debug
- The build MUST complete with no errors and no new warnings before the feature is "done".
- If the generator complains about a mismatch, delete cmake-build-debug and re-run the configure step.

DEFINITION OF DONE (for every prompt)
- Code compiles clean (no errors, no new warnings).
- New parameters appear in the editor and are bound via an APVTS attachment.
- New parameters save and reload with the project (do not special-case state handling).
- No audible regression to existing oscillator/filter/ADSR behaviour.
- New helper .cpp/.h files (if any) are added to target_sources in CMakeLists.txt.
```

---

# Prompt 1 - Filter Envelope + Env -> Cutoff

```text
GOAL
Add a dedicated filter envelope (a second ADSR) to each voice that modulates the filter cutoff
frequency. This is independent of the existing amplitude ADSR.

FILES TO EDIT
- Source/PluginProcessor.cpp  (createParameterLayout + processBlock)
- Source/SynthVoice.h         (new members + updateParameters signature)
- Source/SynthVoice.cpp       (envelope setup, modulation math, render)
- Source/PluginEditor.h       (new sliders + attachments)
- Source/PluginEditor.cpp     (configure sliders + attachments + layout)

NEW PARAMETERS (add to createParameterLayout, copy the style of the existing ATTACK/DECAY params)
- "FILTER_ENV_ATTACK"  "Filter Env Attack"  range 0.001..5.0  skew 0.4  default 0.02
- "FILTER_ENV_DECAY"   "Filter Env Decay"   range 0.001..5.0  skew 0.4  default 0.20
- "FILTER_ENV_SUSTAIN" "Filter Env Sustain" range 0.0..1.0    default 0.50
- "FILTER_ENV_RELEASE" "Filter Env Release" range 0.001..8.0  skew 0.4  default 0.40
- "FILTER_ENV_AMOUNT"  "Filter Env Amount"  range -1.0..1.0   default 0.0   (bipolar)

IMPLEMENTATION
1. In SynthVoice.h add:
     juce::ADSR filterEnv;
     juce::ADSR::Parameters filterEnvParams;
     float filterEnvAmount = 0.0f;
     float baseCutoffHz = 18000.0f;   // cutoff before envelope modulation
2. Extend updateParameters(...) with 5 new trailing args:
     float filterEnvAttack, float filterEnvDecay, float filterEnvSustain,
     float filterEnvRelease, float filterEnvAmount
   Update the declaration in SynthVoice.h, the definition in SynthVoice.cpp, and the single
   call site in PluginProcessor::processBlock so they all match.
3. In updateParameters(): store baseCutoffHz = clampedCutoff; store filterEnvAmount; set
   filterEnvParams and call filterEnv.setParameters(filterEnvParams). Do NOT call
   filter.setCutoffFrequency here for the modulated value - cutoff is now applied per-block in render.
4. In startNote(): call filterEnv.noteOn(). In stopNote(): call filterEnv.noteOff().
5. In prepareToPlay(): filterEnv.setSampleRate(sampleRate) and set sensible default params.
6. In renderNextBlock(): before processing the filter, compute the modulated cutoff for this block:
     const float envValue = filterEnv.getNextSample();   // 0..1, advances the env
     // map bipolar amount to a multiplicative range, e.g. 5 octaves up/down:
     const float octaves = filterEnvAmount * 5.0f * envValue;
     const float modCutoff = juce::jlimit (20.0f, 20000.0f, baseCutoffHz * std::pow (2.0f, octaves));
     filter.setCutoffFrequency (modCutoff);
     ladderFilter.setCutoffFrequencyHz (modCutoff);
   NOTE: getNextSample() advances one sample; for block-rate modulation that is acceptable here,
   but call it once per block (not per sample) to keep it cheap, OR advance it in the per-sample
   loop if you prefer sample-accurate behaviour. Block-rate is fine for v1.

EDITOR
Add 5 rotary sliders ("F.Atk","F.Dec","F.Sus","F.Rel","F.Env") with SliderAttachments to the new
parameter IDs, following the exact pattern already used for ATTACK/DECAY/etc. Update resized() so
they get bounds (increase the window width or add a second row of sliders as needed).

VERIFY
Build clean. With FILTER_ENV_AMOUNT > 0, a held note should open the filter on attack and settle to
the sustain level; with amount < 0 it should close. Amount = 0 must sound identical to the current build.
```

---

# Prompt 2 - LFO 1 (reusable Lfo class)

```text
GOAL
Add one per-voice low-frequency oscillator (LFO 1) with selectable shape, free-run rate, and a single
routable destination (filter cutoff, pitch, or amplitude). Implement it as a small reusable class so
future LFOs can reuse it.

FILES TO EDIT / CREATE
- Source/Lfo.h   (NEW - the reusable LFO class)
- Source/Lfo.cpp (NEW)
- CMakeLists.txt (register Source/Lfo.cpp and Source/Lfo.h in target_sources(Softysn1 ...))
- Source/PluginProcessor.cpp (params + processBlock)
- Source/SynthVoice.h/.cpp (own an Lfo, apply modulation)
- Source/PluginEditor.h/.cpp (controls)

NEW CLASS: Lfo (Source/Lfo.h / .cpp)
Public API (keep it minimal and allocation-free after prepare):
    void prepare (double sampleRate);
    void reset();
    void setShape (int shape);          // 0 Sine, 1 Triangle, 2 Saw, 3 Square, 4 Sample&Hold
    void setRateHz (float hz);
    float getNextSample();              // returns bipolar value in [-1, 1]
Implementation: keep a float phase in [0,1) advanced by rateHz/sampleRate each sample. Compute the
shape from phase. For Sample&Hold, hold a random value (juce::Random) and refresh it when phase wraps.
No heap allocation; everything is plain members.

NEW PARAMETERS (createParameterLayout)
- "LFO1_SHAPE"  "LFO 1 Shape"  AudioParameterChoice { "Sine","Triangle","Saw","Square","S&H" } default 0
- "LFO1_RATE"   "LFO 1 Rate"   range 0.01..40.0 Hz  skew 0.3  default 2.0
- "LFO1_DEPTH"  "LFO 1 Depth"  range 0.0..1.0  default 0.0
- "LFO1_DEST"   "LFO 1 Dest"   AudioParameterChoice { "Off","Cutoff","Pitch","Amp" } default 0

INTEGRATION (SynthVoice)
- Add: Lfo lfo1; plus cached ints/floats lfo1Shape, lfo1Dest, lfo1Depth.
- Extend updateParameters(...) with: int lfo1Shape, float lfo1Rate, float lfo1Depth, int lfo1Dest.
  Update the declaration, definition, and the processBlock call site to match.
- In prepareToPlay: lfo1.prepare(sampleRate). In startNote: lfo1.reset().
- In renderNextBlock, once per block, get float lfoValue = lfo1.getNextSample(); then apply by dest:
    Cutoff: multiply the cutoff by std::pow(2.0f, lfoValue * lfo1Depth * 2.0f) before filter.setCutoffFrequency.
            (If Prompt 1 is merged, modulate baseCutoffHz alongside the filter envelope.)
    Pitch:  set oscillator frequency to baseFrequencyHz * std::pow(2.0f, lfoValue * lfo1Depth * (2.0f/12.0f))
            (i.e. up to +/-2 semitones at full depth). Apply to both osc and wavetableOsc.
    Amp:    scale the gain by (1.0f + lfoValue * lfo1Depth * 0.5f), clamped to >= 0.
  When dest == Off or depth == 0, behaviour must be unchanged.

EDITOR
Add an LFO1 shape combo + dest combo (ComboBoxAttachment) and rate + depth rotary sliders
(SliderAttachment), wired to the new IDs. Update resized().

VERIFY
Build clean. Set LFO1_DEST=Cutoff, raise depth and rate -> hear the filter wobble. Set Dest=Pitch ->
vibrato. Set Dest=Off or Depth=0 -> identical to before.
```

---

# Prompt 3 - Oscillator Unison + Sub-oscillator

```text
GOAL
Thicken the main oscillator with unison (multiple detuned/spread copies) and add a sub-oscillator one
or two octaves below the played note. Applies to the analog oscillator path in SynthVoice.

FILES TO EDIT
- Source/PluginProcessor.cpp (params + processBlock)
- Source/SynthVoice.h/.cpp   (unison voices + sub osc)
- Source/PluginEditor.h/.cpp (controls)

NEW PARAMETERS
- "UNISON_VOICES" "Unison Voices" AudioParameterInt 1..7 default 1
- "UNISON_DETUNE" "Unison Detune" range 0.0..1.0 default 0.2   (0 = none, 1 = ~50 cents spread)
- "UNISON_SPREAD" "Unison Spread" range 0.0..1.0 default 0.5   (stereo width across the unison voices)
- "SUB_LEVEL"     "Sub Level"     range 0.0..1.0 default 0.0
- "SUB_OCTAVE"    "Sub Octave"    AudioParameterChoice { "-1 Oct","-2 Oct" } default 0

IMPLEMENTATION (SynthVoice)
- Replace the single juce::dsp::Oscillator<float> osc with a small fixed-size array of oscillators,
  e.g. std::array<juce::dsp::Oscillator<float>, 7> unisonOscs; and a separate
  juce::dsp::Oscillator<float> subOsc (sine or the selected analog wave).
- In prepareToPlay, prepare all of them. In updateOscillator/initialise, initialise each unison osc
  with the SAME waveform lambda currently used (Sine/Saw/Square/Noise). Keep the Wavetable path as-is
  for now (unison can apply to the analog path only in v1; document that clearly).
- Extend updateParameters(...) with: int unisonVoices, float unisonDetune, float unisonSpread,
  float subLevel, int subOctave. Update declaration/definition/call site.
- In startNote, set each active unison oscillator frequency to baseFrequencyHz detuned by a symmetric
  spread of cents derived from unisonDetune (e.g. evenly spaced in [-detuneCents, +detuneCents]).
  Set subOsc frequency to baseFrequencyHz * 0.5 (octave -1) or * 0.25 (octave -2).
- In renderNextBlock for the analog path: sum the active unison oscillators, divide by the active
  count to keep level sane, pan them across L/R by unisonSpread, then add subOsc * subLevel.
  Keep all scratch buffers pre-allocated in prepareToPlay - do NOT allocate in render.
- When unisonVoices == 1, detune/spread must collapse to the original single-oscillator sound.

EDITOR
Add controls: a unison voices slider (or combo), detune + spread rotaries, sub level rotary, and a
sub octave combo, each with the appropriate attachment. Update resized().

VERIFY
Build clean. Unison 1 == original tone. Increasing voices + detune = wide supersaw-style thickness.
Sub level adds low end; sub octave switches between -1 and -2 octaves.
```

---

# Prompt 4 - Master FX bus (Saturation -> Chorus -> Delay -> Reverb)

```text
GOAL
Add a global (master) effects chain that processes the full stereo mix AFTER the voices are rendered.
This is processor-level DSP (in PluginProcessor), not per-voice.

FILES TO EDIT
- Source/PluginProcessor.h   (FX member objects)
- Source/PluginProcessor.cpp (prepare FX, params, apply FX in processBlock)
- Source/PluginEditor.h/.cpp (controls)

FX CHAIN (use juce::dsp; process in this order on the rendered buffer)
1. Drive/Saturation : juce::dsp::WaveShaper<float> with std::tanh, preceded by a drive gain.
2. Chorus           : juce::dsp::Chorus<float>.
3. Delay            : juce::dsp::DelayLine<float> (stereo, with feedback + mix), or roll a simple
                      stereo delay with two juce::dsp::DelayLine instances. Max delay ~2 seconds.
4. Reverb           : juce::dsp::Reverb.

NEW PARAMETERS (group them clearly by stage)
- "FX_DRIVE_ON" bool default false ; "FX_DRIVE_AMT" range 0..1 default 0.3
- "FX_CHORUS_ON" bool default false ; "FX_CHORUS_RATE" 0.05..10 Hz default 1.0 ;
  "FX_CHORUS_DEPTH" 0..1 default 0.3 ; "FX_CHORUS_MIX" 0..1 default 0.5
- "FX_DELAY_ON" bool default false ; "FX_DELAY_TIME" 0.01..2.0 s default 0.3 ;
  "FX_DELAY_FEEDBACK" 0..0.95 default 0.3 ; "FX_DELAY_MIX" 0..1 default 0.3
- "FX_REVERB_ON" bool default false ; "FX_REVERB_SIZE" 0..1 default 0.5 ;
  "FX_REVERB_DAMP" 0..1 default 0.5 ; "FX_REVERB_MIX" 0..1 default 0.3

IMPLEMENTATION
- In PluginProcessor.h add the four juce::dsp objects as members. In prepareToPlay build a
  juce::dsp::ProcessSpec { sampleRate, samplesPerBlock, getTotalNumOutputChannels() } and call
  prepare() on each. Pre-allocate the delay line's maximum size here (NOT in processBlock).
- In processBlock, AFTER synth.renderNextBlock(...), read the FX params, push them into each FX object
  (setters), then wrap the buffer in a juce::dsp::AudioBlock + ProcessContextReplacing and process
  each enabled stage in order. Bypass a stage cleanly when its *_ON flag is false (skip processing).
- Keep everything allocation-free in processBlock. Use ScopedNoDenormals (already present).

EDITOR
Add a compact FX section: a toggle (juce::ToggleButton + ButtonAttachment) per stage plus the rate/
mix/etc rotaries (SliderAttachment). It is fine to grow the window or add a row. Update resized().

VERIFY
Build clean. Each stage audibly changes the master output only when its toggle is on. With all FX off,
output is identical to the pre-FX build.
```

---

# Prompt 5 - Preset Save / Load (disk + minimal browser)

```text
GOAL
Let users save the full plugin state to a .softpreset file and load it back, with simple
previous/next/save/load controls in the editor. Reuse the existing APVTS XML serialization.

FILES TO EDIT / CREATE
- Source/PresetManager.h / .cpp (NEW - file IO + list of presets)
- CMakeLists.txt (register the new files in target_sources)
- Source/PluginProcessor.h/.cpp (own a PresetManager, expose it)
- Source/PluginEditor.h/.cpp (Save / Load / Prev / Next buttons + current preset label)

PRESET LOCATION
Use juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
  .getChildFile ("Softysn1").getChildFile ("Presets")
Create the directory if it does not exist (do this on the message thread / construction, NOT in audio).

NEW CLASS: PresetManager (Source/PresetManager.h / .cpp)
Holds a reference to the AudioProcessorValueTreeState. API:
    PresetManager (juce::AudioProcessorValueTreeState& apvts);
    void savePreset (const juce::String& name);          // writes apvts.copyState() XML to <name>.softpreset
    void loadPreset (const juce::String& name);           // reads XML, apvts.replaceState(ValueTree::fromXml)
    void loadNext(); void loadPrevious();                 // cycle through the folder
    juce::StringArray getAllPresetNames() const;          // scan the folder for *.softpreset
    juce::String getCurrentPresetName() const;
Implementation reuses the SAME XML approach already in getStateInformation/setStateInformation.
All file IO happens on the message thread (button clicks), never on the audio thread.

INTEGRATION
- In PluginProcessor, add a PresetManager member constructed with the apvts, plus a getter
  PresetManager& getPresetManager().
- DO NOT call preset IO from processBlock.

EDITOR
- Add a juce::Label showing the current preset name, and four juce::TextButtons: "<", ">", "Save",
  "Load". "<" / ">" call loadPrevious/loadNext. "Save" pops a juce::AlertWindow (or a simple text
  entry) for the name then calls savePreset. "Load" can use a juce::FileChooser or load the selected
  name. Refresh the label after any change. Update resized().

VERIFY
Build clean. Save a tweaked sound, change parameters, then Load it back -> the saved values return.
Prev/Next cycle through saved presets. Confirm presets also reload after closing/reopening the plugin.
```

---

# Prompt 6 (Stretch) - Small Modulation Matrix

```text
GOAL
Add a small fixed-size modulation matrix (e.g. 4 slots) that routes modulation SOURCES to parameter
DESTINATIONS with a depth. This generalizes the hard-wired routing from Prompts 1-2. Only attempt this
after Prompt 1 (filter envelope) and Prompt 2 (LFO 1) are merged, because it reuses those modulators.

FILES TO EDIT
- Source/PluginProcessor.cpp (params + processBlock)
- Source/SynthVoice.h/.cpp   (evaluate sources, apply to destinations)
- Source/PluginEditor.h/.cpp (per-slot source/dest combos + depth)

DESIGN (keep it fixed-size and allocation-free)
- Define enums in SynthVoice.h:
    Sources:      Off, Env2 (filter env), LFO1, Velocity, ModWheel
    Destinations: Off, Cutoff, Pitch, Amp, WavetablePosition
- For SLOTS = 4, add per-slot parameters:
    "MOD{n}_SRC"   AudioParameterChoice (source list)   default Off
    "MOD{n}_DEST"  AudioParameterChoice (dest list)     default Off
    "MOD{n}_DEPTH" range -1.0..1.0  default 0.0
  for n in 1..4. Generate these in a loop in createParameterLayout.

IMPLEMENTATION
- Each block, compute the current value of each source in [-1,1] (or [0,1] for velocity/modwheel):
  Env2 from the filter envelope, LFO1 from the Lfo, Velocity from noteVelocity, ModWheel from a stored
  controller value (capture CC1 in controllerMoved).
- For each slot, accumulate source*depth into the matching destination accumulator.
- Apply destination accumulators on top of the base values BEFORE configuring the oscillator/filter/gain
  in renderNextBlock:
    Cutoff -> octave shift on baseCutoffHz ; Pitch -> semitone shift on frequency ;
    Amp -> gain scale ; WavetablePosition -> offset added to the WT position (clamp 0..1).
- All accumulation uses plain floats/arrays. No heap allocation in the audio thread.
- If Prompt 1/2 already hard-wire Env2->Cutoff and LFO1->dest, fold that wiring INTO the matrix so
  there is a single modulation path (avoid double-applying the same modulation).

EDITOR
Add 4 rows, each with a source combo, a dest combo, and a depth rotary, all with attachments.
Update resized() (a dedicated mod-matrix panel/section is fine).

VERIFY
Build clean. Route LFO1 -> WavetablePosition at some depth and hear the wavetable scan. Route Velocity
-> Cutoff and confirm harder notes open the filter. Empty/Off slots have no effect.
```

---

## Appendix - Quick prompt template (for new features beyond this pack)

When you (or the planning model) need a new prompt, copy this skeleton and fill it in:

```text
GOAL
<one or two sentences: what the feature does and where it lives (per-voice vs processor-level)>

FILES TO EDIT / CREATE
<list exact paths; remember to register new .cpp/.h in CMakeLists.txt>

NEW PARAMETERS
<ID, label, type/range, default - copy the style of existing params in createParameterLayout>

IMPLEMENTATION
<follow the 5-step pattern from Block 0; spell out signatures and where to apply DSP>

EDITOR
<which controls + attachments to add; remind to update resized()>

VERIFY
<the observable behaviour that proves it works, plus "neutral settings == previous build">
```
