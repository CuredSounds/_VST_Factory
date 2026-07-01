# VST Factory Operating Guidelines (Sandbox Rule Context)

This file serves as the **Static Context** guidelines for the VST_Factory sandbox directory. Any agent or developer working here must follow these instructions.

## Technology Stack
* C++20
* JUCE Framework (v8.0+)
* CMake (v3.22+)

## Hard DSP & Real-Time Rules (NEVER BREAK)
1. **NO Memory Allocations** inside the real-time audio thread. Do not call `new`, `malloc`, `std::vector::push_back`, or resize vectors/arrays inside `processBlock`. All memory must be pre-allocated in `prepareToPlay()`.
2. **NO Locking or Blocking** calls on the audio thread. Avoid `std::mutex`, semaphores, file IO, socket reads/writes, or `DBG()` console logging.
3. **Parameter Integrity**: All parameters must be read using lock-free atomic values loaded from the APVTS (e.g. using `load()` on the float pointer).
4. **Decoupled Architecture**: All signal processing algorithms go to `Source/DSP/`. Wrapper, presets, and APVTS settings go to `Source/Core/`. Custom graphics go to `Source/UI/`.
5. **No string-literal param IDs**: In new code, use the constants defined in `Source/Core/Parameters.h` (e.g., `VSTFactory::Params::gain`).
