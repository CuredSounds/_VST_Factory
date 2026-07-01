# Will Pirkle SDK — evaluation for Softysn1

References: [willpirkle.com](https://www.willpirkle.com) · ASPiK · SynthLab-DM · *Designing Software Synthesizer Plugins in C++* (2nd ed.)

## What Pirkle offers

| Product | Role |
|---------|------|
| **ASPiK** | Open-source AAX/AU/VST3 plugin kernel; drag-and-drop GUI designer; shared DSP core across formats |
| **SynthLab-DM** | Modular “mini-module” synth lab — compile/load oscillator/filter/LFO blocks without maintaining a full synth project |
| **FX book + code** | 50+ DSP objects (filters, delay, dynamics, pitch, etc.) |
| **Synth book + code** | VA/wavetable/PM building blocks; six complete synth example projects |

## Fit vs current stack (JUCE + custom Bio-Synth)

| Area | Pirkle | Softysn1 today | Recommendation |
|------|--------|----------------|----------------|
| Plugin shell | ASPiK / RackAFX7 | JUCE 8 | **Keep JUCE** — already shipping VST3/AU/Standalone |
| GUI | ASPiK designer | Custom BioSynthUI / JUCE | **Keep custom UI** — Serum-style layout is the differentiator |
| DSP primitives | Book C++ objects | JUCE `dsp` + in-house modules | **Cherry-pick** — port algorithms after A/B, don’t fork stack |
| Wavetable / morph | SynthLab examples | `WavetableOscillator`, editor | Compare morph/interpolation quality in isolated test |
| Filters (VA) | Virtual analog chapter | Ladder + SVF | Good candidate for **reference implementation** audit |
| Mod matrix | Not a turnkey matrix UI | Custom 8-slot matrix | No direct win |

## Suggested “try out” path (low risk)

1. **SynthLab-DM trial** — prototype one block (e.g. ladder filter or LFO) and compare CPU/noise vs existing code.
2. **Read-only port** — pick one class from the synth book (e.g. wave sequencer or morph table) into `Source/DSP/Experimental/Pirkle/` behind a compile flag.
3. **Do not** replace JUCE editor or ASPiK shell mid-project — integration cost > benefit for a branded UI.

## CMake sketch (future, optional)

```cmake
option(SOFTYSN1_PIRKLE_EXPERIMENTS OFF)
if (SOFTYSN1_PIRKLE_EXPERIMENTS)
  # Add vendored Pirkle sources under ThirdParty/pirkle/
  target_compile_definitions(Softysn1 PRIVATE SOFTYSN1_PIRKLE_EXPERIMENTS=1)
endif()
```

## Decision

- **Short term:** Use Pirkle as **DSP reference + SynthLab spikes**, not as the main plugin framework.
- **Revisit** if we need RackAFX-style rapid GUI iteration for internal tools only (not the shipping skin).
