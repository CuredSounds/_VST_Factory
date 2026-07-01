# _VST_Factory — Ecosystem Agent Context

Static context for agents working in this repository. Plugin-specific rules live in
`vst_factory_sandbox/AGENTS.md` when editing the sandbox.

## What this repo is

The _VST_Factory ecosystem: shared specs, harness tooling, and a reference sandbox plugin for
building VST/AU effects and synths with AI agents under the Factory Model.

## Canonical documentation

Read on demand from `_VST_Factory_Claude_docs/`:

- `00_OVERVIEW.md` — factory components map
- `02_ARCHITECTURE_STANDARDS.md` — Core/DSP/UI split (mandatory)
- `03_GUARDRAILS_AND_RT_SAFETY.md` — real-time safety catalog
- `04_TESTING_AND_QUALITY_GATES.md` — CI gates and "done" policy
- `05_HARNESS_AND_AGENT_WORKFLOW.md` — conductor vs orchestrator, model routing

## Workflow

- **Conductor mode** (in-IDE): DSP tuning, debugging, unfamiliar code.
- **Orchestrator mode** (async agents): scaffolding, test generation, multi-file migrations.

Before marking any roadmap item done, link a verification artifact (passing test, profiler log,
CI run). A checkbox without an artifact is not done. See `04_TESTING_AND_QUALITY_GATES.md` §5.

## Scope

- **Active work:** `vst_factory_sandbox/` harness and tooling.
- **Reference only:** `Synth Refrence Build Specs and Docs/`, root `PROJECT_STANDARDS.md`,
  `CHANGELOG.md` (Softsyn1 case study).
- **Softsyn1 code:** [CuredSounds/_vst_softysn1](https://github.com/CuredSounds/_vst_softysn1) — harness on branch `try_vst_factory`

## Hard rules (all plugins)

1. No heap allocation in `processBlock` or on the audio thread.
2. No locking, blocking I/O, or logging on the audio thread.
3. Parameter IDs only in `Source/Core/Parameters.h` — never string literals elsewhere.
4. Include direction: UI → Core → DSP. DSP never includes UI.
5. Build and test DSP before wiring UI.
