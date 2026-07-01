# Architecture & Specification Standards

This is the "Specifications and Context" pillar of the Factory Model: the blueprint an agent
gets before it writes any code. It generalizes the pattern already proven on Softsyn1
(`Reference_specs 1/ARCHITECTURE.md`, `PROJECT_STANDARDS.md`, `docs/GREENFIELD_CHECKLIST.md`)
to any plugin in the _VST_Factory ecosystem, effect or synth.

## 1. The three-layer split (mandatory for every plugin)

```
Source/
├── Core/      # JUCE wrapper: PluginProcessor, PluginEditor, PresetManager, Parameters.h
├── DSP/       # Pure C++ engine. No JUCE GUI headers. Unit-testable in isolation.
│   ├── Engines/<EngineName>/        # synths: one folder per synthesis engine
│   └── EffectModules/<ModuleName>/  # effects: one folder per effect type, registered via factory
└── UI/        # LookAndFeel, Widgets, Panels. No audio processing.
Tests/
└── DSP_Tests/ # Catch2, one test file per DSP module
Presets/
└── Factory/
docs/
└── GREENFIELD_CHECKLIST.md   # the day-1 setup checklist, copied into every new plugin repo
```

Include direction is one-way: **UI → Core → DSP**. DSP code never includes anything from UI.
This is what makes the DSP layer testable without spinning up a plugin host, and it's the
single architectural decision that does the most to keep AI agents from accidentally wiring
GUI state into the audio thread.

## 2. Why this layering is the spec, not just style

Per Day_1_v3's framing: "AI excels at implementing architectural decisions once they are made."
The Core/DSP/UI split *is* the architectural decision. Once an agent is told "the DSP layer has
no UI dependencies and exposes only pure functions/structs," it can scaffold consistent,
conforming code across dozens of effect modules or synth engines without re-deriving the
architecture each time. This is the leverage point of context engineering applied to plugin
development — get this one document right and every subsequent generation task gets cheaper
and more reliable.

## 3. Parameter specification (define every parameter exactly)

Per the Factory Model's "Specifications & Context" pillar, vague prompts produce bad-sounding
artifacts. Every parameter needs, at minimum:

```
Parameter: Filter Cutoff
Range: 20 Hz – 20 kHz
Scale: logarithmic
Default: 1000 Hz
Smoothing: yes — 20ms ramp on the audio thread, recalculated on parameter change, not per-sample
APVTS ID: defined once in Source/Core/Parameters.h, in a single namespace
  (e.g. Softysn1::Params::FilterCutoff) — never a string literal anywhere else,
  and never rebuilt by concatenation inside processBlock.
```

This level of detail is what separates "structured AI-assisted coding" from real agentic
engineering on the Vibe Coding → Agentic Engineering spectrum (Day_1_v3, Table 1): formal specs
and architecture docs instead of casual prompts.

## 4. DSP signal-flow specification

Every effect or synth voice needs an explicit signal-flow diagram in its module's README before
an agent writes the processing code, e.g.:

```
Oscillator -> Non-linear Saturator -> 4-pole Ladder Filter -> Output VCA
```

For synth engines specifically, document: the engine type (VA / wavetable / granular / FM /
additive / spectral / physical-modeling / sample-based), voice allocation strategy and max
polyphony, and how the engine's parameters map into the shared mod matrix.

## 5. Folder and naming conventions (carried forward from Softsyn1)

- All parameter IDs centralized in one header inside one namespace. No exceptions.
- One effect module or synth engine = one folder, registered through a factory/registry pattern
  (`EffectModuleFactory`-style), never a hardcoded `if`/`switch` chain that has to be edited for
  every new module.
- No new `.cpp` files at `Source/` root — everything lives under `Core/`, `DSP/`, or `UI/`.
- CMakeLists.txt references canonical paths only; don't let generated or scratch files leak into
  the build graph.
- Master docs (`ROADMAP.md`, `CHANGELOG.md`) live at the plugin repo root and are the single
  source of truth. Any other doc that talks about roadmap or status (e.g. a `features/*.md` file)
  must redirect to the master doc, not fork a second copy of the backlog. Softsyn1's repo already
  does this correctly in several places (`features/ROADMAP.md` is a one-line redirect) — keep
  that pattern; it's what stops roadmap drift across documents written by different agents in
  different sessions.

## 6. The Day-1 Greenfield Checklist

Softsyn1's `docs/GREENFIELD_CHECKLIST.md` is explicitly written as "a day-1 template for the
next JUCE plugin" — this is the seed of _VST_Factory as a reusable system rather than a one-off
project. Every new plugin (effect or synth) in the ecosystem should start from that checklist:

1. CMake scaffold with FetchContent JUCE 8, CMakePresets.json, VST3/AU/Standalone/CLAP targets
   (CLAP pinned to a commit hash).
2. `Source/Core/Parameters.h` namespace stub before any other code.
3. `Source/DSP/` skeleton with one trivial pass-through module and its Catch2 test, to prove the
   test harness works before real DSP is written.
4. `AGENTS.md` filled in from `01_AGENTS_TEMPLATE.md`.
5. Empty `ROADMAP.md` and `CHANGELOG.md` at repo root.
6. CI wired to run the DSP test suite and a static-analysis pass on every commit before any
   feature work starts (see `04_TESTING_AND_QUALITY_GATES.md`).

Do not let an agent start writing feature DSP before steps 1–6 exist. This ordering is itself a
guardrail: it's much cheaper to fix architecture-by-convention before there are 50 files that
violate it than after.

## 7. Effects processors vs. synthesizers — what differs

The three-layer split, parameter centralization, and module-factory pattern are identical for
both. What differs:

| | Effects processor | Synthesizer |
|---|---|---|
| Voice management | Usually none (mono signal path) or simple stereo | Voice allocation, polyphony limits, unison, MPE |
| Mod sources | Typically host automation only | Internal LFOs/envelopes/mod matrix feeding many destinations |
| State to persist | Plugin parameter state only | Plugin parameter state + per-voice state + possibly sample/wavetable data |
| Heaviest RT-safety risk | Buffer-size-dependent latency reporting, tail length (`getTailLengthSeconds()`) | Voice-stealing logic, mod-matrix evaluation cost scaling with active mod count |

Both share the same guardrail catalog in `03_GUARDRAILS_AND_RT_SAFETY.md` — the synth-specific
risks are additive, not a different rule set.
