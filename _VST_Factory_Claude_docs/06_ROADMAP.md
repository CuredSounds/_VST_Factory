# _VST_Factory Roadmap

Two tracks, run in parallel: standing up the **ecosystem** (the harness, standards, and skills
that make every future plugin faster and safer to build) and putting that ecosystem to its
first real test by **reconciling and stabilizing Softsyn1**, the existing reference build. Don't
sequence these — Softsyn1 is the proving ground for whether the ecosystem docs in this folder
actually hold up, and the ecosystem work is what stops Softsyn1's same mistakes (and the
roadmap-drift problem described below) from recurring on the next plugin.

## Phase 0 — Ecosystem foundation (this deliverable + immediate follow-up)

- [x] Write the Factory Model doc set: `00`–`05` in this folder.
- [ ] Copy `01_AGENTS_TEMPLATE.md` into Softsyn1's repo as its actual `AGENTS.md`, filling in the
      plugin-specific section. Cross-check it against Softsyn1's existing `PROJECT_STANDARDS.md`
      and reconcile any conflicts rather than maintaining two competing standards documents.
- [ ] Wire the CI gate described in `04_TESTING_AND_QUALITY_GATES.md` section 7 into Softsyn1's
      build (allocation profiler, null tests, MIDI stress tests, build-all-formats). This doesn't
      exist yet per the code reviews — its absence is exactly how the processBlock allocation bug
      shipped undetected.
- [ ] Stand up the "done" gate (testing doc, section 5) for `ROADMAP.md`/`CHANGELOG.md` going
      forward: no entry without a linked verification artifact.

## Phase 1 — Reconcile and stabilize Softsyn1 (the proving ground)

This is the most urgent item, not because it blocks the ecosystem work, but because shipping
Softsyn1 in its current state would ship known, convergently-confirmed real-time-safety bugs.

1. **Reconcile the two conflicting roadmap documents.** `Reference_specs 1/ROADMAP.md`
   (2026-06-27, conservative, Sprint 6 "in progress") and
   `Synth Refrence Build Specs and Docs/Softysyn1_Flagship_Roadmap.md` (claims Phases 7–15 100%
   complete) cannot both be right — and the code reviews dated the same day as the latter's
   claims found unfixed P0 bugs. Re-verify every "complete" claim in the Flagship Roadmap against
   an actual build/test artifact before trusting it; treat unverified items as not-done by
   default per the new "done" gate.
2. **Fix the convergent RT-safety findings** from the five 2026-06-29 code reviews, in priority
   order:
   - `processBlock` string-built parameter lookups (~190 allocations/block) — the top-priority
     fix across every review. Move all parameter resolution to parameter-change time.
   - `std::async` per-voice-layer pattern on/near the audio thread — replace with lock-free
     pre-computed state.
   - Duplicate `MidiBuffer` allocation per block.
   - Font static teardown-order crash risk on host exit.
   - File-chooser lambda unsafe `this` capture.
   - Sample/wavetable buffer mutation racing the audio thread.
   - Unsandboxed Lua scripting host.
3. **Re-verify the earlier (2026-06-27) P0 fixes are still in place** after the above changes:
   tail-length reporting, per-voice EffectChain cost, `renderNextBlock` buffer allocation. These
   were fixed once; confirm they haven't regressed during later feature work.
4. **Run the QA listening pass** that `features/todos.md` scaffolds but never completed: actually
   audition all 52–68 factory presets against the checklist and log results, rather than leaving
   the checklist as an unfilled template.
5. **Repo hygiene:** confirm the JUCE-vendored-checkout removal (CHANGELOG notes this happened)
   is complete and `.gitignore` covers build artifacts; pin the CLAP dependency to a commit hash
   if it's still tracking `main`.

Only after steps 1–3 have linked verification artifacts should any "Phase 7–15 complete" claim
be treated as accurate, and only then should net-new feature work (rather than stabilization)
resume on Softsyn1.

## Phase 2 — Generalize the harness across plugins

- [ ] Extract the Day-1 Greenfield Checklist (`02_ARCHITECTURE_STANDARDS.md` section 6) into a
      standalone scaffold script/template repo, so starting plugin #2 is a CLI command, not a
      copy-paste of Softsyn1's structure.
- [ ] Build the Agent Skills described in `05_HARNESS_AND_AGENT_WORKFLOW.md` section 5: new
      effect module, new synth engine, RT-safety audit, null-test golden update. Validate each
      skill against Softsyn1 first since it's the only codebase mature enough to exercise them.
- [ ] Decide and document the second plugin in the ecosystem: per `02_ARCHITECTURE_STANDARDS.md`
      section 7, an effects processor is the cheaper proving ground for the harness (no voice
      allocation/mod-matrix complexity) before attempting a second synth.

## Phase 3 — Quality flywheel at ecosystem scale

- [ ] Formalize ensemble review (testing doc, section 6) as a required gate for any change
      touching the audio thread or voice allocation, across every plugin, not just Softsyn1.
- [ ] Build the observability layer described in the harness doc: a running log across all
      plugins of which model produced which change, so recurring failure classes can be traced
      back to a model/workflow gap rather than treated as one-off bugs.
- [ ] Revisit model routing (harness doc, section 2) with real cost data once two or more
      plugins are generating enough CI/review volume to matter financially — this is the point
      where the "Economics of AI Development" tradeoffs in Day_1_v3 stop being theoretical.
- [ ] Periodically re-audit `03_GUARDRAILS_AND_RT_SAFETY.md` against new bugs found in
      production; the catalog should keep growing from real incidents, the same way Softsyn1's
      `AGENTS.md` rule list is meant to.

## Open questions to resolve before Phase 2 starts

- Is AAX support actually required? It's flagged across multiple Softsyn1 docs as needing Avid
  SDK access and signing that was never completed — worth a deliberate scope decision rather
  than carrying it as permanently "todo."
- Which CI/build infrastructure hosts the gate in Phase 0 — local-only via hooks, or a hosted
  runner? This affects how the allocation-profiler and null-test steps get invoked.
- Does the second plugin share a codebase/monorepo with Softsyn1, or live in its own repo using
  the Phase 2 scaffold template? Recommend separate repos sharing the template, to keep the
  one-way UI→Core→DSP and module-factory conventions enforceable per-repo rather than needing
  cross-plugin coordination.
