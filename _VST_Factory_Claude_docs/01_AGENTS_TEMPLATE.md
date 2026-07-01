# AGENTS.md Template — Static Context for VST/Synth Plugin Agents

Copy this file into the root of any plugin repo (effect or synth) as `AGENTS.md` (and/or `CLAUDE.md`, `GEMINI.md` — keep them identical or symlinked; per Day_1_v3, static context like this is loaded on every interaction, so it must stay short and high-signal). Fill in the bracketed sections per plugin. Everything outside brackets is the standing rule set proven on Softsyn1 and meant to carry forward unchanged.

This file is the agent's **static context**: instructions + guardrails it should never have to be reminded of. Dynamic context — the specific feature spec you're working on today, the current failing test output — belongs in the prompt, not here.

---

```markdown
# [Plugin Name] — Agent Context

## What this plugin is
[One paragraph: effect or synth, target sound/competitor, plugin formats shipped.]

## Tech Stack
* C++20
* JUCE 8 (FetchContent, not vendored — do not commit a JUCE checkout into the repo)
* CMake + Ninja, CMakePresets.json
* Plugin formats: VST3, AU, Standalone, CLAP (clap-juce-extensions, pin to a commit hash, never `main`)
* Catch2 for DSP unit tests (Tests/DSP_Tests, gated by a BUILD_TESTS CMake flag)

## Hard DSP Rules (NEVER BREAK)
1. NO memory allocation in `processBlock` or anywhere on the audio thread: no `new`, no `malloc`,
   no `std::vector` resizing, no `juce::String` construction or concatenation, no `std::map`/
   `std::unordered_map` insertion, no `std::async` / `std::thread` spawning.
2. NO locking or blocking calls on the audio thread: no mutexes, no file I/O, no console/logging
   output, no waiting on futures.
3. Pre-calculate filter coefficients and resolve all parameter-ID-to-value lookups when parameters
   change (on the message thread), NEVER inside the per-sample or per-block loop. Cache the
   resolved values/pointers; the audio thread only reads them.
4. Block-size independence: DSP must work correctly whether the host sends 32, 64, 512, or 4096
   samples per call.
5. Sample-rate independence: all filters, envelopes, and LFOs derive coefficients from
   `getSampleRate()` at `prepareToPlay`/parameter-change time — never a hardcoded 44100/48000.
6. All parameter IDs are centralized in one header (e.g. `Source/Core/Parameters.h`) inside a
   single namespace. No raw string literals for parameter IDs anywhere else in the codebase,
   especially not rebuilt per-call inside `processBlock` (see GUARDRAILS doc, Anti-Pattern #1 —
   this exact mistake cost Softsyn1 ~190 allocations/block and was caught by five independent
   AI reviewers on the same day).
7. Any background work (file loading, wavetable import, preset scanning) must hand off to a
   dedicated non-audio thread and publish results to the audio thread via lock-free structures
   (atomics, single-producer/single-consumer queues) — never a raw `this`-capturing async lambda
   that outlives the object (see GUARDRAILS doc, Anti-Pattern #5).
8. Any embedded scripting (Lua, JS) runs sandboxed with no filesystem/network access by default.
9. Static/global objects that own platform resources (typefaces, fonts, GL contexts) must have an
   explicit, ordered teardown hook — do not rely on static destruction order across translation
   units (see GUARDRAILS doc, Anti-Pattern #4).

## Architecture (do not violate the layering)
* `Source/Core/` — JUCE wrapper layer: PluginProcessor, PluginEditor, PresetManager, Parameters.h.
* `Source/DSP/` — pure, UI-free, unit-testable engine. No JUCE GUI headers. No `Editor` includes.
* `Source/UI/` — LookAndFeel, Widgets, Panels. No audio processing, no APVTS writes from a
  background thread.
* Include direction is one-way: UI → Core → DSP. DSP must never `#include` anything from UI.
* One effect/voice module = one folder under `Source/DSP/EffectModules/<Name>/` or
  `Source/DSP/Engines/<Name>/`, registered through a factory, not a hardcoded switch statement.
* Full layout reference: `02_ARCHITECTURE_STANDARDS.md` in `_VST_Factory_Claude_docs/`.

## Workflow
* Do not design the UI and DSP in the same pass. Build and unit-test the DSP core first.
* Only wire the UI to the AudioProcessorValueTreeState (APVTS) after the DSP passes its tests.
* Before marking any roadmap item "done," it needs a linked verification artifact — a passing
  test run, a profiler trace showing zero allocations in `processBlock`, or a null-test diff.
  A roadmap checkbox with no artifact is not done. See `04_TESTING_AND_QUALITY_GATES.md`.
* When more than one AI agent/model touches the same change, prefer running them as independent
  reviewers on the same diff and look for convergence before trusting a single model's verdict
  (see `04_TESTING_AND_QUALITY_GATES.md`, "Ensemble Review").

## [Plugin-specific section — fill in per project]
* Engines / effect chain specifics:
* Parameter count / mod matrix size:
* Preset format:
* Competitive reference targets:
```

---

## Notes on using this template

- Keep the filled-in `AGENTS.md` under ~150 lines. If a section grows past a paragraph, move the
  detail into a skill file or a doc under `docs/` and link it — static context is the expensive,
  always-loaded kind (per Day_1_v3's static-vs-dynamic context framing). The agent should pull the
  deep reference only when a task actually calls for it.
- Add a rule to the Hard DSP Rules list every time an agent does something it shouldn't do again.
  That is the entire maintenance model for this file — it grows from real incidents, not
  speculative ones.
- If the plugin uses a second AI tool's convention file (`GEMINI.md`, `CLAUDE.md`), keep them
  byte-identical to `AGENTS.md` or symlink them, so the harness doesn't drift between agents.
