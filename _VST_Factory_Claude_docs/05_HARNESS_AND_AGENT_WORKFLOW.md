# Harness & Agent Workflow

Per Day_1_v3: **Agent = Model + Harness**. The model is the engine; the harness — instructions,
tools, sandboxes, orchestration, hooks, observability — is the car, the road, and the traffic
laws. Most agent failures, examined honestly, are configuration failures in the harness, not
model limitations. This doc defines the harness for _VST_Factory and formalizes a workflow
pattern Softsyn1 was already running informally (`OLLAMA_PROMPTS.md`, `Init_prompt.md`).

## 1. Conductor vs. orchestrator mode, mapped to plugin work

| Mode | Use for | Tools |
|---|---|---|
| **Conductor** (real-time, in-IDE, hands-on) | Tuning a specific DSP algorithm ("make this distortion sound more like analog tape"), debugging a specific crash, working in unfamiliar/legacy code where every change needs to be understood as it's made | In-editor chat/inline completion (Claude Code, Cursor, Gemini Code Assist) |
| **Orchestrator** (async, goal-level, multi-agent) | Scaffolding a new plugin from the Greenfield Checklist, generating a full test suite for an existing module, migrating a codebase pattern across all effect modules, running the ensemble review in `04_TESTING_AND_QUALITY_GATES.md` | Background/terminal agents (Claude Code, Antigravity CLI, Codex CLI, Jules) |

Most plugin work moves between both in a single session: orchestrate the scaffold, then
conduct the tricky filter math by hand. Pick the mode by task risk and ambiguity, not by habit.

## 2. The hybrid frontier + local model pattern (already validated on Softsyn1)

Softsyn1's `OLLAMA_PROMPTS.md` workflow is a working instance of intelligent model routing
(Day_1_v3's "Economics of AI Development" section) applied to plugin dev, worth keeping as the
ecosystem standard:

1. A frontier model (Claude/GPT-class) handles architecture, ambiguous specification, and
   writes a **tightly-scoped prompt** for one feature.
2. A smaller/local model (Kimi, Qwen, etc. via Ollama) executes that one prompt: one prompt =
   one feature = one build.
3. Every feature has an explicit **Definition of Done** gate before the next prompt is issued:
   build clean, no new compiler warnings, no regression in the existing test suite.
4. The frontier model reviews the result before integrating it, applying the guardrail catalog
   in `03_GUARDRAILS_AND_RT_SAFETY.md`.

This is intelligent model routing in practice: expensive reasoning (architecture, ambiguous
tradeoffs, review) goes to the frontier model; cheap, well-specified mechanical work goes to the
smaller model. Use this pattern by default for routine module/effect implementation work, and
reserve full frontier-model attention for architecture decisions, guardrail design, and ensemble
review.

## 3. What's in the harness, concretely, for a _VST_Factory plugin

- **Instructions/rule files:** `AGENTS.md` (from `01_AGENTS_TEMPLATE.md`), plus this doc set
  linked from it for on-demand deep reference.
- **Tools:** the build system (CMake/Ninja) invoked as a tool the agent can run and read output
  from; the test runner (Catch2); a profiler invocation for the allocation/CPU gate; a DAW or
  plugin-validator (e.g. `pluginval`) for load-testing built binaries.
- **Sandboxes:** agents build and test inside a sandboxed environment with file-system access
  scoped to the plugin repo, not the whole machine — especially important once Lua/scripting
  hosts are involved (guardrail #8).
- **Orchestration logic:** when to hand a task to the local model vs. keep it with the frontier
  model (section 2); when to fan a diff out to multiple models for ensemble review (testing doc,
  section 6).
- **Guardrails/hooks:** a pre-commit or pre-merge hook that runs the allocation profiler and
  null tests automatically — don't depend on an agent remembering to run them manually.
- **Observability:** CI logs, profiler traces, and a running log of which model produced which
  change, so that when a bug surfaces later you can tell whether it came from a frontier-model
  architecture decision or a local-model mechanical implementation — these have different fixes.

## 4. Feedback loops (routing failures back to the agent)

- Compiler error → fed back to the model verbatim, automatically, by the orchestration layer.
  Don't paraphrase the error for the agent; raw terminal output is the highest-fidelity feedback.
- DSP test failure (wrong audio output, NaN detected) → feedback loop should explicitly direct
  the agent to check edge cases: division by zero, denormals, uninitialized state at
  `prepareToPlay`, sample-rate-zero or block-size-zero calls.
- Allocation-profiler failure → feedback loop should point the agent at
  `03_GUARDRAILS_AND_RT_SAFETY.md` section B directly; this exact failure mode (string-built
  parameter lookups) has already occurred once and is the single most likely recurrence.
- Roadmap/status claim without a linked artifact → treated as a process failure, not a code
  failure; the fix is enforcing the "done" gate in `04_TESTING_AND_QUALITY_GATES.md` section 5,
  not re-prompting the agent to "try again."

## 5. Skills and reusable context

As the ecosystem grows past one plugin, package recurring procedures as Agent Skills
(progressive-disclosure, loaded only when a task matches) rather than growing `AGENTS.md`
indefinitely:

- A "new effect module" skill: scaffolds a `Source/DSP/EffectModules/<Name>/` folder, its
  Catch2 test stub, and its factory registration, following section 5 of
  `02_ARCHITECTURE_STANDARDS.md`.
- A "new synth engine" skill: same, under `Source/DSP/Engines/<Name>/`, plus mod-matrix wiring.
- An "RT-safety audit" skill: runs the full guardrail catalog against a diff or a module,
  producing a report formatted like the five Softsyn1 code reviews.
- A "null-test golden update" skill: regenerates golden reference audio after an intentional
  sound change, with a mandatory human-reviewed diff summary attached.

Keep each skill scoped to one job. The value of skills, per Day_1_v3, is that the agent stays a
lightweight generalist day-to-day and only pays the context cost for the specific skill a task
calls for.
