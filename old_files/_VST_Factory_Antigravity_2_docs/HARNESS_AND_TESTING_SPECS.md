# Harness & Testing Specifications

To achieve Agentic Engineering, we must wrap the AI models in a harness that enforcing strict quality gates. The VST Factory relies on the following testing and feedback loops.

## 1. Automated Quality Gates

### 1.1 DSP Unit Testing (Headless)
- **Framework:** `Catch2` integrated via CMake (`SOFTYSN1_BUILD_TESTS=ON`).
- **Scope:** Tests must execute DSP classes in isolation without loading the JUCE `AudioProcessor` or GUI.
- **Agent Interaction:** Agents must write a unit test for every new DSP module (e.g., an LFO, a Filter) before integrating it into the `SynthVoice`.

### 1.2 Null Testing (Correctness Verification)
- **Concept:** Process a known `.mid` file through the VST state offline, export a `.wav`, and invert its phase against a "golden" reference `.wav`. If they cancel out completely (absolute silence), the test passes.
- **Usage:** Used to detect regressions in core math (e.g., verifying that optimizing the FM6 algorithm didn't alter the sound).

### 1.3 CPU Profiling & Bounds
- **Concept:** Track the microseconds taken to execute `processBlock`.
- **Thresholds:** If processing 512 samples takes longer than `X` microseconds, the build is flagged. This prevents agents from silently introducing `O(n^2)` algorithms (like naive modulation matrix scans) into the audio thread.

### 1.4 MIDI Stress Testing
- **Concept:** Send thousands of simultaneous Note On, Note Off, Pitch Bend, and CC messages into the engine.
- **Verification:** Ensure voice stealing works flawlessly without memory leaks, hanging notes, or segmentation faults.

## 2. Feedback Loops

When an agent is assigned a task (Orchestrator Mode), the harness handles errors autonomously:
1. **Compilation Failures:** C++ syntax errors and missing headers are automatically scraped from CMake/Ninja output and returned to the agent with instructions to resolve dependencies.
2. **Test Failures:** If a DSP test generates NaN (Not a Number) or Inf outputs, the harness returns the failure to the agent with a prompt to investigate divide-by-zero or denormalization issues.
3. **Guardrail Violations:** A static analyzer or pre-commit hook checks for strings like `new`, `std::vector`, or `std::mutex` inside `Source/DSP/` code. If found, the commit is blocked and the agent is instructed to refactor using pre-allocation or atomics.
