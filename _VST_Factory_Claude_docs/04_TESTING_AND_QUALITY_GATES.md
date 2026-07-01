# Testing & Quality Gates

This is the Factory Model's "Tests and Verification" pillar — the mechanism that turns vague
"does it seem to work?" vibes into the kind of verification that lets you tell a CTO (or
yourself, six months later) that this plugin was built under agentic engineering discipline,
not vibe coding. Per Day_1_v3: tests verify the deterministic parts (function in, output out);
evals verify the non-deterministic parts (did the agent take a reasonable path, does the result
meet a quality bar). Audio plugins need both, plus two categories specific to audio.

## 1. Null testing (audio correctness)

Run a fixed audio file through the DSP module and diff the output, sample-for-sample or within
a tight epsilon, against a stored "golden" reference. Required for:
- Any change to a filter, oscillator, or envelope implementation.
- Any refactor that touches the DSP layer, even if "no behavior change" is intended — null tests
  are exactly how you catch the cases where it wasn't actually a no-op.

A null test failing is not automatically a regression — it might mean the golden file needs
updating because the sound intentionally changed. But it must be *reviewed*, never silently
overwritten by an agent.

## 2. CPU / allocation profiling (the gate that would have caught the 190-allocation bug)

Every DSP module and every full-signal-chain integration test runs under a profiler that fails
the build if:
- `processBlock` (or any function called from it) allocates heap memory. This is checkable
  mechanically — wrap test runs with an allocation-counting allocator or a tool like
  Heaptrack/Instruments, and assert zero allocations after `prepareToPlay`.
- Per-block processing time exceeds a defined microsecond budget for the target sample rate and
  block size (set the budget per module based on its place in the signal chain, not a single
  global number).

This single gate, run as CI rather than left to manual review, is what would have caught
Softsyn1's `processBlock` string-allocation bug before five different AI reviewers had to find
it independently after the fact. Manual/AI code review is a second line of defense, not the
first.

## 3. MIDI stress testing (synths and any effect with MIDI-reactive behavior)

Feed the engine a barrage of: rapid note-on/note-off (faster than voice release time), pitch
bend sweeps, CC automation bursts, sustain-pedal edge cases, and MPE per-note expression if
supported. Assert: no dropped notes beyond the documented polyphony limit, no leaked voices
(voice count returns to zero after all notes off), no NaN/Inf in the output buffer.

## 4. Output evaluation vs. trajectory evaluation

Per Day_1_v3's distinction: output evaluation checks the final artifact (does it compile, do
tests pass); trajectory evaluation checks *how* the agent got there. For plugin work,
trajectory evaluation means: did the agent check `getTailLengthSeconds()` before claiming a
reverb feature complete? Did it run the allocation profiler before marking a DSP change "done,"
or did it just eyeball the diff? A fluent diff that skipped its own verification step is a more
dangerous failure than one that left a visible compile error — the Softsyn1 roadmap-vs-reality
gap (Phases 7–15 marked "complete" the same day reviewers found unfixed RT-safety bugs) is
exactly this failure mode at the project-tracking level, not just the code level.

## 5. The "done" gate for roadmap/status tracking

No item in `ROADMAP.md` or `CHANGELOG.md` moves to done without a linked verification artifact:

| Claim | Required artifact |
|---|---|
| "Feature X implemented" | Passing unit test(s) covering X, linked by name/path |
| "processBlock is allocation-free" | Profiler trace/log showing zero allocations, attached or linked |
| "No regression from refactor Y" | Null-test diff against the golden reference, attached or linked |
| "MPE/voice-stealing works correctly" | MIDI stress test run output, attached or linked |
| "Plugin builds and loads in [DAW]" | Actual build log + host-load confirmation, not just "should work" |

This single rule is the direct fix for the failure mode observed in Softsyn1's own history:
an aspirational roadmap document and a code-review-confirmed bug list disagreeing about the
same codebase on the same day. Status tracking is itself something AI agents will happily
hallucinate completion on if nothing forces a check — treat it with the same rigor as the code.

## 6. Ensemble review (formalizing what already worked)

Softsyn1's `processBlock` bug was independently found by Claude, ChatGPT, Gemini, and Kimi, all
reviewing the same diff without coordinating. That convergence is a usable QA technique, not
just a lucky accident:

- For any change touching the audio thread, signal flow, or voice allocation, run the diff
  through more than one model/agent as an independent reviewer before merging.
- Look for **convergence**: if two or more independent reviewers flag the same root cause
  without prompting each other, treat it as a high-confidence finding regardless of which model
  said it first.
- Disagreement between reviewers is itself useful signal — it usually means the issue is
  genuinely ambiguous (a style/architecture tradeoff) rather than a clear-cut bug, and warrants
  human judgment rather than another AI tiebreaker.
- This is more expensive in tokens than single-model review (see harness doc for model-routing
  guidance on when this is worth the cost) — reserve it for audio-thread-touching and
  voice-allocation-touching changes, not every commit.

## 7. CI wiring (minimum viable setup, Phase 0 of the roadmap)

1. Build the plugin (all formats: VST3, AU, CLAP, Standalone) on every push.
2. Run the Catch2 DSP test suite.
3. Run the allocation/CPU profiler against `processBlock` for at least one representative preset
   per engine/effect type.
4. Run null tests against golden audio references.
5. Fail the build (not just warn) on any of: compile warning regression, allocation in
   `processBlock`, null-test diff outside tolerance, CPU budget exceeded.

Until this exists for a given plugin, treat every "done" claim about that plugin's audio
correctness as unverified, independent of how confident the authoring agent sounded.
