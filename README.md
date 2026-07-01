# _VST_Factory

[![Sandbox CI](https://github.com/CuredSounds/_VST_Factory/actions/workflows/sandbox-ci.yml/badge.svg)](https://github.com/CuredSounds/_VST_Factory/actions/workflows/sandbox-ci.yml)

Agentic engineering ecosystem for building VST/AU audio plugins under the **Factory Model**
(specifications, agents, tests, feedback loops, guardrails).

## Quick start

```bash
cd vst_factory_sandbox
cmake --preset ci
cmake --build --preset ci
./tools/run_dsp_tests.sh
```

Install local hooks (one-time):

```bash
git config core.hooksPath .githooks
```

## Documentation (canonical)

| Doc | Purpose |
|-----|---------|
| [_VST_Factory_Claude_docs/00_OVERVIEW.md](_VST_Factory_Claude_docs/00_OVERVIEW.md) | What this repo is |
| [_VST_Factory_Claude_docs/01_AGENTS_TEMPLATE.md](_VST_Factory_Claude_docs/01_AGENTS_TEMPLATE.md) | Copy into each plugin repo as `AGENTS.md` |
| [_VST_Factory_Claude_docs/02_ARCHITECTURE_STANDARDS.md](_VST_Factory_Claude_docs/02_ARCHITECTURE_STANDARDS.md) | Core/DSP/UI layering |
| [_VST_Factory_Claude_docs/03_GUARDRAILS_AND_RT_SAFETY.md](_VST_Factory_Claude_docs/03_GUARDRAILS_AND_RT_SAFETY.md) | Real-time safety rules |
| [_VST_Factory_Claude_docs/04_TESTING_AND_QUALITY_GATES.md](_VST_Factory_Claude_docs/04_TESTING_AND_QUALITY_GATES.md) | CI gates and "done" policy |
| [_VST_Factory_Claude_docs/05_HARNESS_AND_AGENT_WORKFLOW.md](_VST_Factory_Claude_docs/05_HARNESS_AND_AGENT_WORKFLOW.md) | Agent workflow |
| [_VST_Factory_Claude_docs/06_ROADMAP.md](_VST_Factory_Claude_docs/06_ROADMAP.md) | Ecosystem roadmap |
| [_VST_Factory_Claude_docs/07_PRESET_SMOKE_TESTING.md](_VST_Factory_Claude_docs/07_PRESET_SMOKE_TESTING.md) | pluginval + preset smoke |

## Repository layout

```
_VST_Factory/
├── _VST_Factory_Claude_docs/   # Canonical factory spec (00–07)
├── vst_factory_sandbox/        # Reference plugin + harness proving ground
├── Synth Refrence Build Specs and Docs/  # Softsyn1 case study (reference only)
├── old_files/                  # Superseded doc generations
├── AGENTS.md                   # Ecosystem static context for agents
└── ROADMAP.md                  # Harness implementation tracker
```

## Reference build

**BC_Softsyn / Softsyn1** (`CuredSounds/_vst_softysn1`) is the external flagship synth that
informed this factory. Stabilization of Softsyn1 is **deferred** until the harness in
`vst_factory_sandbox/` is green in CI.

## Starting plugin #2

1. Fork/copy `vst_factory_sandbox/` structure.
2. Fill in `AGENTS.md` from `01_AGENTS_TEMPLATE.md`.
3. Wire CI using the same `tools/run_all_gates.sh` pattern.
