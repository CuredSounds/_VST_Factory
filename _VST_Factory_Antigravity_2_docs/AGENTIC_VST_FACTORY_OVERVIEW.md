# The Agentic VST Factory: System Overview

## 1. Introduction
The development of VST audio effects and software synthesizers is notoriously complex, requiring strict real-time constraints, advanced mathematics, and intricate state management. The **Agentic VST Factory** workflow applies modern "Agentic Engineering" principles (as outlined in the Day 1 SDLC whitepaper) to audio development. 

This workflow marks a transition from ad-hoc "vibe coding" to structured **Agentic Engineering**. You are no longer just writing C++ syntax; you are building the *factory* (the system, constraints, and tests) that produces the code, while AI agents act as the engines on the assembly line.

## 2. From Syntax to Intent
In the VST Factory Model, the developer's role shifts from primary implementor to system designer and quality arbiter. The workflow relies on two primary modes of operation:
- **The Orchestrator Mode (Asynchronous):** For large, well-defined tasks (e.g., "Scaffold a new JUCE synthesizer with an APVTS and oscillator module"). The developer assigns tasks to background agents, reviews the output, and provides course corrections.
- **The Conductor Mode (Synchronous):** For complex DSP logic or fine-tuning (e.g., "Tune this saturation algorithm to sound more like analog tape"). The developer works in-IDE, guiding the agent's mathematical changes in real-time.

## 3. The 5 Pillars of the VST Factory

A raw AI model cannot build a stable VST on its own; it requires a "Harness" to execute tasks safely. The formula is **Agent = Model + Harness**. Our VST factory consists of five core components:

### 3.1. Specifications and Context (The Blueprint)
Vague prompts lead to terrible sounding audio artifacts. Before execution, the intent must be defined:
- **Parameter Specs:** Exact definitions (e.g., "Filter Cutoff: 20Hz to 20kHz, log scale, default 1kHz, smoothing applied").
- **DSP Architecture:** Clear signal flow mapping (e.g., "Oscillator -> Non-linear Saturator -> 4-pole Ladder -> VCA").
- **UI/UX Intent:** Visual layout and parameter mapping.

### 3.2. Guardrails (The Factory Safety Sensors)
Hard constraints that keep the agent's behavior safe and prevent DAW crashes. These are embedded in the static context (`AGENTS.md`):
- **Audio Thread Safety:** No memory allocation, no locks, no file I/O in the `processBlock`.
- **Block Size Independence:** DSP must handle 32, 64, or 1024 samples dynamically.
- **Sample Rate Independence:** Coefficients must calculate based on the host's current sample rate.

### 3.3. Agents & The Harness (The Assembly Line)
The harness wraps the AI model, providing it with tools, sandboxes, and execution environments. In the VST Factory, the harness ensures that code is generated within the established Wrapper/Engine/UI architectural boundaries.

### 3.4. Tests and Verification (Quality Control)
Verification goes beyond "Does it compile?". The factory requires automated quality gates:
- **Null Testing:** Run audio through the DSP and compare against a "golden" output to check phase/amplitude.
- **CPU Profiling:** Fail tests if processing time exceeds strict microsecond thresholds per block.
- **MIDI Stress Testing:** Bombard the synth with Note On/Off and CC data to ensure voice allocation stability.

### 3.5. Feedback Loops (Automated Correction)
When tests fail, the harness routes the failure back to the agent:
- Compiler errors (C++ syntax, JUCE API changes) are fed back automatically.
- DSP test failures (e.g., NaN outputs, clipping) trigger self-correction prompts focusing on edge cases like denormals or divide-by-zero errors.
