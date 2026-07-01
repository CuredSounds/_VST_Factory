# VST Factory Sandbox — Agent Context

## What this plugin is

Reference **gain/bypass effect** plugin for the _VST_Factory harness. Proves Core/DSP/UI
layering, Catch2 tests, CI gates, and preset smoke tests before building plugin #2.

## Tech Stack

* C++20
* JUCE 8.0.3 (FetchContent — do not vendor JUCE in-repo)
* CMake + Ninja, `CMakePresets.json` (`dev` / `ci`)
* Formats: VST3, AU, Standalone
* Catch2 for DSP unit tests (`BUILD_TESTS=ON`)

## Hard DSP Rules (NEVER BREAK)

1. NO memory allocation in `processBlock` or on the audio thread.
2. NO locking or blocking on the audio thread.
3. Pre-resolve parameters; audio thread reads atomics only.
4. Block-size and sample-rate independence.
5. All parameter IDs in `Source/Core/Parameters.h` only.
6. Background work via lock-free handoff — no unsafe async lambdas.
7. No embedded scripting in this sandbox.

## Architecture

* `Source/Core/` — PluginProcessor, Parameters.h
* `Source/DSP/EffectModules/GainProcessor/` — pure gain DSP
* `Source/UI/` — PluginEditor
* Include direction: UI → Core → DSP

## Workflow

* Run `tools/run_all_gates.sh` before marking work done.
* Deep reference: `../_VST_Factory_Claude_docs/`

## Plugin-specific

* Single effect: linear gain + bypass
* Competitive reference: none (harness scaffold only)
