# Preset Smoke Testing & Pre-DAW Validation

This extends `04_TESTING_AND_QUALITY_GATES.md` with a concrete, scriptable answer to: *how do
we test all the DSP and controls automatically, in the build environment, before a plugin ever
loads in a real DAW* — and how do we make "no sound" mean "found a bug" instead of "forgot to
turn a knob."

Three layers, each catching a different failure class. Run all three in CI; none replaces
another.

| Layer | Catches | Knows about your engines? |
|---|---|---|
| A. **pluginval** | Host-compatibility crashes, hangs, state save/restore corruption, generic parameter fuzzing, thread-safety at high strictness | No — fully generic |
| B. **Preset smoke tests** (your idea, formalized) | "This preset should make sound and doesn't" / NaN / silence / clipping, per engine or effect | Yes — driven by baseline presets |
| C. **Parameter sweep fuzzer** | NaN/Inf/denormal-driven CPU spikes when a single control is pushed to an extreme, independent of preset | Partially — iterates registered params generically but renders against a real preset context |

## Layer A — pluginval (the generic pre-DAW gate)

[pluginval](https://github.com/Tracktion/pluginval) is a cross-platform, open-source plugin
validator built for exactly this purpose: load the actual built VST3/AU/CLAP binary headlessly
and put it through the paces a DAW would, without needing a DAW. It returns exit code 0/1, so it
gates CI directly.

```bash
# tools/run_pluginval.sh
PLUGINVAL_STRICTNESS=8   # 5 = minimum host-compat baseline; 8-10 adds param fuzzing + repeated state restore
pluginval \
  --validate "build/Softysn1_artefacts/VST3/Softysn1.vst3" \
  --strictness-level "$PLUGINVAL_STRICTNESS" \
  --skip-gui-tests \
  --timeout-ms 30000 \
  --output-dir build/pluginval-logs
echo "pluginval exit code: $?"
```

Wire this as a required CI step. At strictness 8+ it already fuzzes parameters and restores
state repeatedly — this is a second, independent line of defense against exactly the class of
bug the Softsyn1 code reviews found (it won't catch the specific allocation-in-processBlock
issue, but it will catch state-corruption and crash-on-fuzz issues in the same family).

## Layer B — Baseline preset smoke tests (this is your idea, made concrete)

### The manifest schema

Every synthesis engine and every effect module gets one (or a few) baseline preset(s) with a
manifest sitting next to it, so the *expected* behavior is documented and versioned, not just
assumed. This is what turns "no sound" from a scavenger hunt into a pass/fail assertion.

```
Tests/PresetSmokeTests/
├── FM6_Baseline.xml              # actual plugin state (APVTS ValueTree export)
├── FM6_Baseline.smoketest.yaml   # manifest: what "correct" means for this preset
├── Wavetable_Baseline.xml
├── Wavetable_Baseline.smoketest.yaml
├── Granular_Baseline.xml
├── Granular_Baseline.smoketest.yaml
├── VA_Unison_Baseline.xml
├── ... one pair per engine (VA, Wavetable, Granular, Sampler, FM6, Additive,
│       Spectral, Vector, Karplus-Strong/Modal, String Machine, Vowel/Formant, Organ)
│       and per major effect module (reverb, delay, distortion, comb/formant filter, etc.)
```

```yaml
# FM6_Baseline.smoketest.yaml
preset_file: FM6_Baseline.xml
description: >
  6-operator FM, algorithm 4, all operator levels and ratios set to audible defaults,
  no mute/solo left on, output gain at unity. Exists so a silent render means the FM
  engine broke, not that a tester forgot to raise operator 3's level.
test_note: 60          # MIDI note (C3)
note_velocity: 100
note_duration_sec: 2.0
render_tail_sec: 1.0   # extra render time to catch envelope/reverb tail issues
sample_rate: 48000
block_size: 512
assertions:
  no_nan_or_inf: true
  min_rms_dbfs: -40      # fails if the render is at/near silence
  max_peak_dbfs: -0.1    # fails if it's clipping straight out of the box
  min_spectral_energy_above_hz: 1500   # optional: sanity-check FM sidebands exist at all
    # (skip this assertion for engines where it doesn't apply)
  max_cpu_us_per_block: 150   # flags a perf regression on this specific preset, not just in the abstract
notes: >
  If this fails with silence: check operator routing/algorithm index before assuming
  a DSP regression — algorithm 4 must route at least one carrier to output.
```

The `notes:` field matters as much as the assertions — it's where you write down the "don't
waste an hour on the wrong knob" knowledge you already have in your head, so it survives past
this session.

### Running it — two implementation options

**Option 1: Python + DawDreamer (fastest to stand up, tests the real shipped binary)**

[DawDreamer](https://github.com/DBraun/DawDreamer) is a Python-wrapped, JUCE-based offline
render host — it loads your actual built VST3/AU, sets its state from a preset, sends MIDI,
renders to a numpy array, and lets you assert on the audio in plain Python/pytest. This is
almost exactly the tool shape you're describing, and it needs zero DAW.

```python
# Tests/PresetSmokeTests/test_preset_smoke.py
import glob, yaml, numpy as np
import dawdreamer as daw

PLUGIN_PATH = "build/Softysn1_artefacts/VST3/Softysn1.vst3"
SAMPLE_RATE = 48000
BLOCK_SIZE = 512

def render_preset(manifest):
    engine = daw.RenderEngine(SAMPLE_RATE, BLOCK_SIZE)
    plugin = engine.make_plugin_processor("synth", PLUGIN_PATH)
    plugin.load_state(manifest["preset_file"].replace(".yaml", ".xml"))
    plugin.add_midi_note(manifest["test_note"], manifest["note_velocity"],
                          0.0, manifest["note_duration_sec"])
    graph = [(plugin, [])]
    engine.load_graph(graph)
    engine.render(manifest["note_duration_sec"] + manifest["render_tail_sec"])
    return engine.get_audio()  # shape: (channels, samples)

def test_all_baseline_presets():
    for manifest_path in glob.glob("Tests/PresetSmokeTests/*.smoketest.yaml"):
        manifest = yaml.safe_load(open(manifest_path))
        audio = render_preset(manifest)

        assert not np.isnan(audio).any(), f"{manifest_path}: NaN in output"
        assert not np.isinf(audio).any(), f"{manifest_path}: Inf in output"

        rms_dbfs = 20 * np.log10(np.sqrt(np.mean(audio**2)) + 1e-12)
        peak_dbfs = 20 * np.log10(np.max(np.abs(audio)) + 1e-12)

        assert rms_dbfs > manifest["assertions"]["min_rms_dbfs"], (
            f"{manifest_path}: silent or near-silent render "
            f"({rms_dbfs:.1f} dBFS) — {manifest.get('notes', '')}"
        )
        assert peak_dbfs < manifest["assertions"]["max_peak_dbfs"], (
            f"{manifest_path}: clipping at default settings ({peak_dbfs:.1f} dBFS)"
        )
```

Run with `pytest Tests/PresetSmokeTests/ -v` — one line of red output per broken engine, naming
the exact preset and manifest note, instead of a silent plugin and a guessing game.

**Option 2: In-process C++/Catch2 (no Python toolchain, reuses your existing test harness)**

If you'd rather not add a Python dependency to the build, the same idea runs inside the Catch2
suite you already have, using JUCE's own `AudioPluginFormatManager` to instantiate the real
`AudioProcessor` (not just the DSP-only classes) as a black box:

```cpp
// Tests/PresetSmokeTests/PresetSmokeTests.cpp
TEST_CASE("Preset smoke tests", "[smoke]") {
    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();

    for (auto& manifestPath : findSmokeTestManifests("Tests/PresetSmokeTests")) {
        auto manifest = loadManifest(manifestPath); // your own tiny YAML/JSON reader

        std::unique_ptr<juce::AudioPluginInstance> plugin =
            instantiateBuiltPlugin(formatManager); // load the just-built VST3/AU
        plugin->prepareToPlay(manifest.sampleRate, manifest.blockSize);
        loadPresetState(*plugin, manifest.presetFile);

        juce::AudioBuffer<float> buffer(plugin->getTotalNumOutputChannels(), manifest.blockSize);
        juce::MidiBuffer midi;
        midi.addEvent(juce::MidiMessage::noteOn(1, manifest.testNote, (juce::uint8)manifest.velocity), 0);

        double totalRms = 0.0;
        int numBlocks = (int)((manifest.noteDurationSec + manifest.tailSec) * manifest.sampleRate / manifest.blockSize);
        for (int b = 0; b < numBlocks; ++b) {
            plugin->processBlock(buffer, midi);
            midi.clear();
            if (b == 1) midi.addEvent(juce::MidiMessage::noteOff(1, manifest.testNote), 0);

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                auto* data = buffer.getReadPointer(ch);
                for (int s = 0; s < buffer.getNumSamples(); ++s) {
                    REQUIRE_FALSE(std::isnan(data[s]));
                    REQUIRE_FALSE(std::isinf(data[s]));
                    totalRms += data[s] * data[s];
                }
            }
        }
        double rmsDbfs = 20.0 * std::log10(std::sqrt(totalRms / (numBlocks * buffer.getNumSamples())) + 1e-12);
        INFO(manifest.notes);
        REQUIRE(rmsDbfs > manifest.minRmsDbfs);
    }
}
```

**Recommendation:** start with Option 1 (DawDreamer) — it's faster to stand up, tests the exact
binary you ship, and the YAML-manifest-driven approach is a natural fit for pytest. Add Option 2
later only if you want zero non-C++ dependencies in CI.

## Layer C — Parameter sweep fuzzer (tests "all the controls," not just the preset defaults)

Baseline presets prove a *known-good* configuration works. They don't prove every control is
safe across its whole range. Add a fuzzer that, per baseline preset, iterates every APVTS
parameter across min/25%/50%/75%/max while continuously rendering:

```python
# Tests/PresetSmokeTests/test_param_sweep.py
def test_param_sweep_no_nan_or_dropout():
    for manifest_path in glob.glob("Tests/PresetSmokeTests/*.smoketest.yaml"):
        manifest = yaml.safe_load(open(manifest_path))
        engine, plugin = make_engine_and_plugin(manifest)
        for param in plugin.get_parameters_description():
            for value in [0.0, 0.25, 0.5, 0.75, 1.0]:
                plugin.set_parameter(param["index"], value)
                audio = render_preset(manifest, engine, plugin)
                assert not np.isnan(audio).any(), f"{param['name']} @ {value}: NaN"
                assert not np.isinf(audio).any(), f"{param['name']} @ {value}: Inf"
```

This is a synth-domain-specific complement to pluginval's own built-in parameter fuzz test
(Layer A at strictness 8+) — pluginval doesn't know which preset context makes a parameter
audible, so pairing it with a baseline preset gets you real coverage instead of fuzzing silence.

## Wiring it into CI (extends `04_TESTING_AND_QUALITY_GATES.md` §7)

```
1. Build plugin (all formats)
2. Catch2 DSP unit tests               <- pure DSP layer, no host
3. Preset smoke tests (Layer B)        <- NEW: catches "no sound"/NaN per engine
4. Parameter sweep fuzzer (Layer C)    <- NEW: catches per-control edge cases
5. pluginval at strictness 8+ (Layer A)<- generic host-compat + state/thread safety
6. Null tests against golden references
```
Fail the build on any step. A preset smoke test failure names the exact engine/effect and
points at the manifest's `notes:` field — that's the mechanism that replaces "track down all
the knobs" with "read the one line that says what was already known about this preset."

## Updating the "done" gate

Add one row to the table in `04_TESTING_AND_QUALITY_GATES.md` §5:

| Claim | Required artifact |
|---|---|
| "Engine/effect X implemented and audible" | A baseline preset + manifest for X exists under `Tests/PresetSmokeTests/`, and it passes in CI |

An engine isn't "done" until it has a baseline preset that a machine checks on every build —
not just a factory preset a human liked the sound of once.
