# _VST_Factory: An Agentic Engineering Ecosystem for VST Plugins

## What this is

_VST_Factory is the system that builds VST/AU/CLAP audio plugins — effects processors and software synthesizers — using AI coding agents under the **Factory Model** of agentic engineering (Osmani/Saboo/Kartakis, *The New SDLC with Vibe Coding*, Day_1_v3, Google, May 2026).

The core idea of the Factory Model: your output as the developer is no longer the C++ — it's the *system that produces* the C++. That system has five parts, and this doc set gives you one artifact per part:

| Factory component | Doc in this set |
|---|---|
| Specifications & Context | [`02_ARCHITECTURE_STANDARDS.md`](02_ARCHITECTURE_STANDARDS.md) |
| Agents (the assembly line) | [`05_HARNESS_AND_AGENT_WORKFLOW.md`](05_HARNESS_AND_AGENT_WORKFLOW.md) |
| Tests & quality gates | [`04_TESTING_AND_QUALITY_GATES.md`](04_TESTING_AND_QUALITY_GATES.md) |
| Feedback loops | [`05_HARNESS_AND_AGENT_WORKFLOW.md`](05_HARNESS_AND_AGENT_WORKFLOW.md) (Feedback Loops section) |
| Guardrails | [`03_GUARDRAILS_AND_RT_SAFETY.md`](03_GUARDRAILS_AND_RT_SAFETY.md) |

Plus a static-context template — [`01_AGENTS_TEMPLATE.md`](01_AGENTS_TEMPLATE.md) — that gets copied as `AGENTS.md`/`CLAUDE.md` into every individual plugin repo, and a build sequence — [`06_ROADMAP.md`](06_ROADMAP.md) — for standing the whole ecosystem up.

## Why this exists now

You already have a working proof of concept: **BC_Softsyn / Softsyn1** (repo `CuredSounds/_vst_softysn1`), a flagship hybrid synth (VA/wavetable/granular/FM6/additive/spectral/physical-modeling engines, 16-slot mod matrix, Omnisphere-style 8-part Multi mode) built in JUCE 8 / C++20, benchmarked against Serum, Avenger, Phase Plant, and Omnisphere. It already has a real `PROJECT_STANDARDS.md`, `ARCHITECTURE.md`, and `docs/GREENFIELD_CHECKLIST.md` — and that checklist literally says it's "a day-1 template for the next JUCE plugin." That sentence is the seed of _VST_Factory: you already intended to build more than one plugin from this scaffold.

That same project also produced the most important input to this doc set: **five independent AI code reviews (Claude ×2, ChatGPT, Gemini, Kimi), all dated 2026-06-29, that converged on the identical finding** — `processBlock` was doing ~190 heap allocations per audio block from `juce::String` concatenation in parameter lookups, plus `std::async` calls on the audio thread. Every reviewer found the same root cause independently. That convergence is direct, empirical proof that (a) the guardrails in [`03_GUARDRAILS_AND_RT_SAFETY.md`](03_GUARDRAILS_AND_RT_SAFETY.md) are the *right* ones to enforce mechanically rather than trust to review, and (b) multi-model ensemble review is a real, working quality gate worth formalizing rather than running once informally.

The same set of documents also surfaced a second lesson, just as important: `Softysyn1_Flagship_Roadmap.md` claims Phases 7–15 are 100% complete (full MPE, undo/redo, CLAP shipped) — dated the *same day* as code reviews that found unfixed, ship-blocking real-time-safety bugs. This is a textbook case of an AI-generated status document drifting from verified ground truth because nothing forced a build/test gate before the checkbox got marked. [`04_TESTING_AND_QUALITY_GATES.md`](04_TESTING_AND_QUALITY_GATES.md) and [`06_ROADMAP.md`](06_ROADMAP.md) both build in an explicit fix for this: no roadmap item moves to "done" without a linked, reproducible verification artifact (a passing test run, a profiler trace, a null-test diff).

## How the pieces fit together

```
_VST_Factory/                          <- the ecosystem repo (this one)
├── _VST_Factory_Claude_docs/          <- you are here: the factory's own spec
│   ├── 00_OVERVIEW.md
│   ├── 01_AGENTS_TEMPLATE.md          <- copy into every plugin repo as AGENTS.md
│   ├── 02_ARCHITECTURE_STANDARDS.md   <- Core/DSP/UI pattern, reusable across plugins
│   ├── 03_GUARDRAILS_AND_RT_SAFETY.md <- hard rules + real anti-pattern catalog
│   ├── 04_TESTING_AND_QUALITY_GATES.md
│   ├── 05_HARNESS_AND_AGENT_WORKFLOW.md
│   └── 06_ROADMAP.md
├── Reference_specs 1/                 <- BC_Softsyn project docs (case study / source material)
└── Synth Refrence Build Specs and Docs/  <- BC_Softsyn build specs, code reviews (case study)
```

Each individual plugin (Softsyn1 today; future effects processors and synths) is its own repo or sub-project that imports the `01_AGENTS_TEMPLATE.md` static context and follows the `02`–`04` standards. _VST_Factory itself is where the shared harness, skills, and roadmap for *producing more plugins faster and safer* live.

## Who reads what

- **Starting a new plugin (effect or synth):** read `01`, `02`, `03` in order, then run the Day-1 checklist in `06_ROADMAP.md` Phase 0.
- **Already mid-build and hunting a bug class:** go straight to `03_GUARDRAILS_AND_RT_SAFETY.md` — it's organized as a catalog of real failure modes, not just abstract rules.
- **Setting up CI / review process:** `04_TESTING_AND_QUALITY_GATES.md`.
- **Deciding when to drive (conductor) vs. delegate (orchestrator), or which model to assign to which task:** `05_HARNESS_AND_AGENT_WORKFLOW.md`.
- **Planning what to build next, for the ecosystem or for Softsyn1 specifically:** `06_ROADMAP.md`.
