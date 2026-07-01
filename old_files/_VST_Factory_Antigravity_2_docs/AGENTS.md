# VST Factory Context

## Tech Stack
* **Language:** C++20
* **Framework:** JUCE Framework (v7+)
* **Build System:** CMake
* **Plugin Formats:** VST3, AU, Standalone (Optional: AAX, CLAP)

## Architecture Boundaries
The codebase strictly separates the wrapper, engine, and UI to ensure DSP can be unit-tested and reused without a GUI.
- **Wrapper (`Source/Core/`):** `AudioProcessor`, APVTS layout, `PresetManager`, `Parameters.h`.
- **Engine (`Source/DSP/`):** Voices, oscillators, filters, mod matrix, and effect modules. NO UI elements.
- **UI (`Source/UI/`):** Look-and-feel, panels, widgets. NO audio processing logic.
- **Tests (`Tests/DSP_Tests/`):** Headless Catch2 tests linking only `Source/DSP/`.

## Hard DSP Rules (NEVER BREAK)
The `processBlock` function is the most critical path. Violating these rules will crash the DAW or cause audio dropouts:
1. **NO Memory Allocation:** Do not use `new`, `malloc`, or resize `std::vector` or `juce::Array` in the audio thread. Pre-allocate all memory in `prepareToPlay`.
2. **NO Locking:** Do not use mutexes, locks, or blocking calls (like file I/O or console printing) on the audio thread. Use atomic variables or lock-free FIFOs for cross-thread communication.
3. **Pre-calculate Coefficients:** Calculate filter and envelope coefficients when parameters change (control rate), NOT inside the per-sample audio loop.
4. **Sample Rate Independence:** Always calculate time-based DSP (filters, delays, LFOs) using the host's current sample rate provided in `prepareToPlay`. Never hardcode 44.1kHz.
5. **Block Size Independence:** Your DSP loop must process `buffer.getNumSamples()` correctly, whether it is 32, 64, or 1024 samples.

## Workflow Rules
* Do not attempt to design the UI and the DSP in the same pass. 
* Build the DSP core first and ensure it passes basic headless audio tests.
* Only map the UI to the `AudioProcessorValueTreeState` (APVTS) after the DSP logic is verified.
* All APVTS string IDs must be centralized in `Source/Core/Parameters.h` to prevent automation mismatches.
