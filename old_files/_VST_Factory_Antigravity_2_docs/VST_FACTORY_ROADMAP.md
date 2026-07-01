# Roadmap: Agentic VST Factory & Flagship Synthesizer

This roadmap outlines the dual goals of (1) building the Agentic VST Factory ecosystem (the "harness" and tooling) and (2) utilizing that ecosystem to complete the Softysyn1 / Bio-Synth flagship synthesizer.

---

## Part 1: Ecosystem & Factory Buildout (The Harness)

To scale from "vibe coding" to production-grade "Agentic Engineering," we must invest in the factory's CapEx: the context, skills, and testing infrastructure.

### Phase 1: Context & Static Scaffolding
- [ ] Establish `AGENTS.md` as the source of truth for all VST agents (tech stack, hard DSP rules, architecture boundaries).
- [ ] Define dynamic context profiles (Agent Skills) for specialized tasks: e.g., a "DSP Math Expert" skill vs. a "JUCE UI Developer" skill.
- [ ] Standardize the project structure (Wrapper / Engine / UI / Tests) to allow agents to work in isolated domains.

### Phase 2: Quality Gates & Verification (The Tests)
- [ ] **Headless DSP Unit Testing Framework:** Expand the `Catch2` setup in `Tests/DSP_Tests/` to support automated agent-driven execution.
- [ ] **Automated Null Testing:** Build scripts that bounce a fixed MIDI file through the compiled VST and compare the output waveform against a known-good reference.
- [ ] **CPU & Memory Profiling Hooks:** Implement automated performance tracking to ensure agents do not introduce heavy, unoptimized DSP (e.g., O(n^2) algorithms in the audio thread).

### Phase 3: Automated Feedback Loops
- [ ] **CI/CD Agent Integration:** Wire test failures (NaNs, dropouts, compiler errors) directly back to the agent for autonomous correction (Trajectory Evaluation).
- [ ] **Strict Code Review Prompts:** Deploy an evaluator agent that reviews all generated code specifically for audio thread violations (locks, allocations, print statements).

---

## Part 2: Softysyn1 Flagship Synthesizer (The Output)

With the factory online, agents will be orchestrated to execute the remaining phases of the Softysyn1 roadmap (Phases 7-15).

### Phase 7: Critical Infrastructure
- **Agent Focus:** Conductor mode (real-time guidance).
- **Tasks:** Undo/Redo integration, DSP unit test expansion, FM6 adjacency pre-computation, configurable polyphony (up to 64 voices).

### Phase 8 & 9: Analog Character & Modulation Expansion
- **Agent Focus:** Orchestrator mode (isolated DSP algorithm design).
- **Tasks:** Implement thermal drift, filter saturation, envelope analog overshoot, envelope followers, Sample & Hold, and Chaos/Attractor modulators. *Verify via Null Testing.*

### Phase 10: Effects Suite Completion
- **Agent Focus:** Orchestrator mode (factory pattern generation).
- **Tasks:** Generate DSP modules for Flanger, Phaser, Pitch Shifter, Harmonizer, Convolution Reverb, and Lo-Fi FX. Map UI via `EffectSlotUI`.

### Phase 11 & 12: Advanced Engines & Sequencing
- **Agent Focus:** Hybrid Orchestrator / Conductor.
- **Tasks:** Build Additive (64-partial), Spectral (STFT), and Vector engines. Implement Euclidean rhythm generators, step probability, and polyphonic note sequencing.

### Phase 13 & 14: Expression, MPE, and UI Polish
- **Agent Focus:** Orchestrator mode (UI state binding).
- **Tasks:** Finalize full MPE (Slide/Pressure), MIDI Learn surface, comprehensive tooltips, patch randomizer, and theme toggles.

### Phase 15: Performance Optimization
- **Agent Focus:** Conductor mode (surgical, highly-reviewed adjustments).
- **Tasks:** SIMD (AVX2) oscillator banks, multi-threaded voice processing (`std::async`), and control-rate LFO processing. *Strictly gated by CPU profiling tests.*
