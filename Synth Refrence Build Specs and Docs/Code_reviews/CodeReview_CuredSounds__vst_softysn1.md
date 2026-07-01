# ChatGPT 5.5 High

Yes — I found the private GitHub repo `CuredSounds/_vst_softysn1`, and I reviewed the main CMake setup, processor, Lua host, sampler/wavetable loading, and arpeggiator path. I did **not** run a local build, so this is a static review, not a compiler-backed audit.

## **Overall verdict**

This is ambitious and already has the bones of a serious synth: JUCE, multi-part architecture, modular FX, APVTS, Lua scripting, presets, telemetry, sample zones, wavetable import, CLAP/VST3/AU/Standalone targets, and Catch2 tests. But right now it has the classic “one-man synth castle” smell: powerful, sprawling, and one bad thread crossing away from a DAW crash at 2 a.m.

The biggest risks are:

1. **Too much happening inside `processBlock()`**  
2. **Real-time safety problems**  
3. **Heavy repository bloat**  
4. **State/preset/audio-thread boundary confusion**  
5. **Header-heavy architecture that may slow builds and hide coupling**

---

## **1\. Repository hygiene is the first fire to put out**

Your repo is huge: about **205 MB**, and it contains a full `JUCE-master` checkout plus `cmake-build-debug` artifacts. The search results show both `JUCE-master/...` files and generated build files under `cmake-build-debug/...` committed into the repo.  

That is not fatal, but it is mud in the fuel line.

**Recommendation:**

Add a proper `.gitignore`, remove generated build folders from Git, and choose one dependency strategy:

cmake-build-\*/  
build/  
out/  
DerivedData/  
.vscode/  
.idea/  
\*.user  
\*.log  
\*.dSYM/  
.DS\_Store

For JUCE, I would **not** commit `JUCE-master` unless you intentionally want a vendored framework. Your current CMake already supports either a local JUCE checkout or `FetchContent` fallback.

Better options:

git rm \-r \--cached cmake-build-debug  
git rm \-r \--cached JUCE-master

Then either use:

FetchContent\_Declare(  
    juce  
    GIT\_REPOSITORY https://github.com/juce-framework/JUCE.git  
    GIT\_TAG 8.0.3  
)

or use a Git submodule:

git submodule add https://github.com/juce-framework/JUCE.git external/JUCE

Right now the repo is carrying its workshop, toolbox, sawdust, and lunch wrappers. Keep the machine, lose the wrappers.

---

## **2\.**

## **`processBlock()`**

## **is doing too much**

`processBlock()` is enormous. It copies a large APVTS parameter set into `activePatch()` every block, handles MIDI learn, harmonizer, sequencer, pitch tracking, drum sequencing, step modulation, arpeggiator sync, shared resource sync, rendering, FX, envelope following, random modulation, Lorenz/Turing modulation, and telemetry.

That will become hard to reason about, hard to profile, and hard to make real-time safe.

**Recommendation: split the audio engine into stages:**

void Softysn1AudioProcessor::processBlock (juce::AudioBuffer\<float\>& buffer,  
                                           juce::MidiBuffer& midi)  
{  
    juce::ScopedNoDenormals noDenormals;

    beginBlock();  
    syncParametersForAudioBlock();  
    processMidiControllers(midi);  
    processMidiGenerators(midi, buffer.getNumSamples());  
    updateModulators(buffer, midi);  
    renderSynth(buffer, midi);  
    processEffects(buffer);  
    updateTelemetry(buffer);  
}

Then create separate units:

Source/Core/AudioBlockContext.h  
Source/Core/ParameterSync.cpp  
Source/Core/MidiPipeline.cpp  
Source/Core/ModulationRuntime.cpp  
Source/Core/RenderPipeline.cpp

This is not just “clean code” theater. In audio plugins, clarity is stability. The audio thread is a moving train; you do not remodel the dining car while it is crossing a bridge.

---

## **3\. Audio-thread safety: this needs discipline**

### **Red flag: deferred preset loading inside**

### **`processBlock()`**

At the top of `processBlock()`, you call:

applyDeferredPresetIfReady();

Later, `applyDeferredPresetIfReady()` calls preset-related deferred loading and checks a pending FX preset flag.

That may be okay only if the deferred work is strictly lock-free, allocation-free, file-I/O-free, and deterministic. But the name is suspicious. Anything involving preset loading, files, XML, JSON, ValueTrees, or allocations should generally stay off the audio thread.

**Recommendation:**

Use a double-buffered state handoff:

struct PendingAudioState  
{  
    std::atomic\<bool\> ready { false };  
    std::shared\_ptr\<const CompiledPatch\> patch;  
};

Message thread prepares the full state. Audio thread atomically swaps a pointer at block boundary. No disk I/O. No XML parsing. No heap churn.

---

## **4\. Sample and wavetable loading may race the audio thread**

`loadSampleFileForZone()` loads into `sampleZoneBuffers`, then calls `syncVoiceMatrixSamples()`, which writes pointers into the active patch.

`loadWavetableFile()` writes into `userWavetable`, then sets `activePatch().wavetableDataPtr = &userWavetable`.

This is conceptually right, but dangerous if voices are reading those buffers while the editor thread mutates/replaces them. Atomics on the `loaded` flags do not protect the underlying buffer contents or pointer lifetime.

**Recommendation:**

Use immutable shared sample/wavetable objects:

struct SampleZoneData  
{  
    juce::AudioBuffer\<float\> buffer;  
    double sampleRate \= 44100.0;  
    juce::String name;  
};

std::atomic\<std::shared\_ptr\<const SampleZoneData\>\> zone1;

Or for a JUCE-safe simpler route, use a `std::shared_ptr<const SampleZoneData>` swapped under a small lock **outside** the audio callback, then copied atomically into a render-side pointer at block boundary.

Do not let the UI thread write into memory the audio thread is reading. That is how a synth becomes a haunted house.

---

## **5\. Lua scripting is powerful, but sandbox it harder**

The Lua host opens the standard Lua libraries with `luaL_openlibs(L)`.

That gives scripts access to more than you probably want inside a plugin. Also, `setParam()` calls `beginChangeGesture()`, `setValueNotifyingHost()`, and `endChangeGesture()` directly from Lua.

For a plugin, scripting should be treated like a live wire.

**Recommendations:**

Do **not** run arbitrary Lua scripts on the audio thread. From your header comment, it sounds like you intend message-thread only, which is good.

Consider replacing `luaL_openlibs()` with a restricted standard library set. At minimum, restrict `io`, `os`, package loading, file access, and unbounded loops.

Add a command queue:

struct ScriptCommand  
{  
    juce::String paramId;  
    float normalisedValue;  
};

juce::AbstractFifo scriptCommandFifo;

Lua writes commands. The message thread applies them. The audio thread never executes Lua.

---

## **6\. Arpeggiator: decent start, but timing is not host-musical yet**

The arpeggiator uses `rateHz`, derives `stepSamples`, clears incoming MIDI, builds a pattern, and emits notes.

That works for a prototype, but for a real instrument you probably want beat divisions, PPQ sync, transport awareness, and note-off cleanup when disabling.

Current behavior also hardcodes output channel 1 for arp notes.

**Recommendations:**

Use host PPQ/BPM for musical timing:

enum class RateDivision  
{  
    quarter,  
    eighth,  
    sixteenth,  
    eighthTriplet,  
    sixteenthTriplet  
};

Track source MIDI channel, or allow a selected output channel.

When disabling arp, emit note-offs for active generated notes before reset. Stuck notes are the oldest curse in MIDI country.

---

## **7\. Parameter sync needs a table-driven system**

You are manually copying many APVTS values into `activePatch()` every block. It is explicit, which is good for early debugging, but this will become brittle as the synth grows.

A better structure:

struct FloatBinding  
{  
    const char\* paramId;  
    std::atomic\<float\> VoiceParameterMatrix::\* target;  
};

static constexpr FloatBinding bindings\[\] \=  
{  
    { Softysn1::Params::attack, \&VoiceParameterMatrix::attack },  
    { Softysn1::Params::decay,  \&VoiceParameterMatrix::decay  },  
    { Softysn1::Params::sustain,\&VoiceParameterMatrix::sustain },  
};

Then:

for (const auto& b : bindings)  
{  
    if (auto\* raw \= apvts.getRawParameterValue (b.paramId))  
        (activePatch().\*(b.target)).store (raw-\>load(), std::memory\_order\_relaxed);  
}

This makes it much easier to audit missing parameters and add tests that say: “Every APVTS parameter either maps to DSP or is intentionally UI-only.”

---

## **8\. CMake is good, but needs more production boundaries**

Good signs:

You use C++20, export compile commands, support local JUCE or fetched JUCE, define plugin formats, embed fonts, and have Catch2 tests.

But a few fixes:

Set CLAP dependency to a pinned commit or tag, not `main`. Current CMake fetches `clap-juce-extensions` from `main`, which means tomorrow’s build may not equal today’s build.

Add warnings:

target\_compile\_options(Softysn1 PRIVATE  
    $\<$\<CXX\_COMPILER\_ID:Clang,AppleClang\>:-Wall \-Wextra \-Wpedantic \-Wconversion\>  
)

Turn tests off by default once builds get heavy:

option(SOFTYSN1\_BUILD\_TESTS "Build DSP unit tests" OFF)

Add separate targets later:

Softysn1\_Core  
Softysn1\_DSP  
Softysn1\_UI  
Softysn1\_Plugin  
Softysn1\_Tests

The plugin target should not own the whole kingdom.

---

## **9\. State saving looks promising, but version it now**

`getStateInformation()` copies APVTS state, adds multi-state, adds master FX state, converts to XML, then stores it.

Add explicit version metadata immediately:

state.setProperty ("schemaVersion", 1, nullptr);  
state.setProperty ("pluginVersion", ProjectInfo::versionString, nullptr);

Then make `setStateInformation()` resilient:

const auto schema \= state.getProperty ("schemaVersion", 0);  
if (schema \< 1\)  
    migrateLegacyState (state);

You will thank yourself when old presets from version 0.1 do not explode in version 0.7.

---

## **Priority fix list**

### **Do first**

1. Remove `cmake-build-debug` and generated files from Git.  
2. Decide whether JUCE is submodule, vendored, or `FetchContent`; do not mix casually.  
3. Split `processBlock()` into pipeline functions.  
4. Move preset/sample/wavetable mutation away from direct audio-thread-visible memory.  
5. Pin CLAP dependency instead of using `main`.

### **Do next**

1. Build a parameter binding table.  
2. Add state schema versioning.  
3. Add pluginval to CI/local test flow.  
4. Add voice-stealing tests, stuck-note tests, preset load tests, and sample reload tests.  
5. Harden Lua sandbox and command queue.

### **Do later**

1. Separate DSP/UI/core into libraries.  
2. Add benchmark tests for oscillator, filter, FX, sampler, wavetable, and full patch render.  
3. Add golden audio render tests.  
4. Add crash-resilient telemetry only outside the hot path.

---

## **My blunt take**

You are building something much closer to **Omnisphere-meets-Serum-meets-Falcon** than a simple first VST. That is cool. It is also dangerous. The code already wants to become a cathedral, but the foundation still needs drainage.

The good news: the architecture is not garbage. There is a real design trying to emerge: APVTS → patch matrix → multi-engine → modular FX → UI panels. Keep that spine. But cut the brush around it, because right now `processBlock()` is carrying too much civilization on its back.

# Kimi2.6

Here's a thorough review of your [*vst\_softysn1*](https://github.com/CuredSounds/_vst_softysn1) repository — a JUCE-based VST synthesizer with an impressively ambitious scope.

---

## **Overall Architecture**

The project has a solid, well-organized structure with a clear `Source/DSP/` hierarchy separating concerns like `OscillatorModule`, `LfoModule`, `EffectChain`, and `ModulationBus` . The separation of DSP, UI, and Core directories is good practice. However, several `.h`\-only "stub" files (e.g., `ChorusModule.h`, `DelayModule.h`, `ReverbModule.h` at 58–68 bytes) appear to be placeholders with no implementation yet . These should either be marked `// TODO` clearly or tracked as open issues.

---

## **`WavetableOscillator.cpp` — Key Issues**

**`wrapPhase` uses a while-loop instead of `fmod`** :

cpp  
*`// Current (slow for large values)`*  
`while (value >= 1.0f) value -= 1.0f;`  
`while (value < 0.0f)  value += 1.0f;`

*`// Better`*  
`return value - std::floor(value);`

Using `std::floor` is branchless and significantly faster in a hot audio path.

**`buildTables` uses `std::pow` in a hot loop** for the folded waveform :

cpp  
`folded += (1.0f / std::pow(n, 1.25f)) * harmonicSample;`

`std::pow` is expensive. Since this runs at initialization time it won't kill performance, but precomputing the rolloff coefficient in a lookup table would be cleaner and safer if `buildTables` ever gets called at runtime.

**Duplicate `#include` in `SynthVoice.h`** — `OscillatorModule.h` is included twice :

cpp  
`#include "DSP/OscillatorModule.h"  // line ~13`  
`#include "DSP/OscillatorModule.h"  // line ~19 — redundant`

Remove the duplicate; `#pragma once` prevents a double-parse but it's still messy.

---

## **`Lfo.cpp` — Issues & Improvements**

**S\&H (`case 4`) only updates at cycle wrap** . This means the very first sample after `trigger()` plays the stale `heldValue = 0.0f`. Fix by immediately sampling on trigger when shape is S\&H:

cpp  
`case TriggerMode::Retrig:`  
    `phase = phaseOffset;`  
    `if (shape == 4) heldValue = random.nextFloat() * 2.0f - 1.0f;`  
    `break;`

**`case 2` saw wave is not bandlimited** :

cpp  
`case 2: output = 2.0f * phase - 1.0f; break; // Saw — aliasing at audio rates`

For LFO use (\<20 Hz) this is fine, but if the LFO rate can reach audio frequencies (which some synths support), you'll get aliasing. Consider adding a PolyBLEP correction or at minimum clamping the maximum rate.

**`shape` is a raw `int` with no bounds checking** . A stray value would silently fall through the switch's `default: break` and return `output = 0.0f` with no warning. Using a scoped `enum class LfoShape` would enforce correctness at compile time.

---

## **`SynthVoice.h` — Structural Concerns**

**Four separate `LfoModule` members instead of an array** :

cpp  
`LfoModule lfoModule;`  
`LfoModule lfo2Module;`  
`LfoModule lfo3Module;`  
`LfoModule lfo4Module;`

This is inflexible and verbose. Prefer:

cpp  
`std::array<LfoModule, 4> lfoModules;`

This makes iteration, serialization, and future extension much easier.

**`isPrepared` flag without thread-safety annotation** . In JUCE, `prepareToPlay` runs on the message thread while `renderNextBlock` runs on the audio thread. Accessing `isPrepared` without `std::atomic<bool>` or a memory fence is a data race. Either use `std::atomic<bool> isPrepared` or assert in `renderNextBlock` only in debug builds.

---

## **Build / Project Issues**

**`cmake-build-debug` directory is committed to the repo** . Build artifacts should not be tracked. Add `cmake-build-*/` to `.gitignore`.

**`compile_commands.json` is committed** . This is a generated file that will be stale for other developers. It belongs in `.gitignore` as well.

**`SaturationModule.pdf` is tracked in the DSP source folder** . Binary documentation files shouldn't live alongside source code — move it to `docs/` or a wiki.

---

## **Quick-Win Summary**

| Issue | File | Priority |
| ----- | ----- | ----- |
| Duplicate `#include` | `SynthVoice.h` | Low |
| `wrapPhase` while-loop | `WavetableOscillator.cpp` | Medium |
| 4× LfoModule → `std::array` | `SynthVoice.h` | Medium |
| `isPrepared` data race | `SynthVoice.h` | High |
| S\&H stale value on trigger | `Lfo.cpp` | Medium |
| Build artifacts in git | `.gitignore` | Medium |
| Stub headers with no impl | `DSP/*.h` | Low (track as issues) |

Would you like me to focus a deeper dive on any specific module — for example the modulation bus architecture, the effect chain, or the granular engine?

# Gemini Pro

Thanks for uploading the files from your cpu\_improvements branch\! I can see the code clearly now.  
You have built a massive, incredibly feature-rich synth engine. The sheer amount of functionality (MPE, Euclidean sequencers, Turing machines, 4 layers per voice, granular, wavetable) is wildly impressive.  
However, reviewing these updated files reveals that the **critical real-time audio violations are still present**. If you try to run this in a DAW like Ableton or Logic, you will experience severe CPU spiking, audio dropouts, and potential crashes when automating parameters or playing heavily polyphonic chords.  
Here is a targeted breakdown of exactly what is causing the CPU issues and how to fix them.

### **1\.** PluginProcessor::processBlock **is Allocating Memory & Doing String Lookups**

This is the single biggest CPU killer in your current codebase. In PluginProcessor::processBlock (which runs on the real-time audio thread), you are dynamically constructing juce::String objects and doing dictionary string lookups (apvts.getRawParameterValue()).  
**The Problem:**  
C++  
// Inside PluginProcessor::processBlock()  
for (int m \= 0; m \< 12; \++m) {  
    // 1\. Allocates heap memory for "MACRO"  
    // 2\. Allocates heap memory to convert m+1 to a string  
    // 3\. Allocates heap memory to concatenate them  
    const auto id \= "MACRO" \+ juce::String (m \+ 1);  
      
    // 4\. Does an O(N) or O(log N) string-matching search through a map/dictionary  
    const float v \= apvts.getRawParameterValue (id)-\>load();  
    // ...  
}

You are doing this for Macros, LFO Custom Waves, Mod Matrix slots, and Drum Sequencer steps. That is **hundreds of string allocations and dictionary lookups per block**.  
**The Fix:**  
You must cache the pointers returned by getRawParameterValue *once* in the constructor or in prepareToPlay, and only call \-\>load() on those raw pointers in the audio thread.  
*In* PluginProcessor.h*:*  
C++  
std::array\<std::atomic\<float\>\*, 12\> macroPointers { nullptr };  
std::array\<std::atomic\<float\>\*, DrumSequencer::kMaxSteps\> drumStepPointers { nullptr };  
// Add arrays for LFO custom waves, mod matrix slots, etc.

*In* PluginProcessor *Constructor (after the APVTS is initialized):*  
C++  
for (int m \= 0; m \< 12; \++m) {  
    macroPointers\[m\] \= apvts.getRawParameterValue("MACRO" \+ juce::String(m \+ 1));  
}  
for (int i \= 0; i \< DrumSequencer::kMaxSteps; \++i) {  
    drumStepPointers\[i\] \= apvts.getRawParameterValue("DRUM\_STEP\_" \+ juce::String(i));  
}

*In* PluginProcessor::processBlock*:*  
C++  
// ZERO memory allocation, ZERO string lookups. Just instant pointer deference.  
for (int m \= 0; m \< 12; \++m) {  
    if (macroPointers\[m\] \!= nullptr) {  
        const float v \= macroPointers\[m\]-\>load();  
        // apply your switch statement here  
    }  
}

### **2\.** SynthVoice::renderNextBlock **is Still Resizing Buffers**

Buffer allocation (setSize) maps down to OS kernel calls (malloc/free / calloc). You cannot do this on the audio thread.  
**The Problem:**  
C++  
// Inside SynthVoice::renderNextBlock()  
if (layerScratch.getNumChannels() \!= tempBuffer.getNumChannels()  
    || layerScratch.getNumSamples() \< numSamples)  
    layerScratch.setSize (tempBuffer.getNumChannels(), numSamples, false, false, true); 

for (auto& buf : layerAsyncScratch) {  
    if (buf.getNumChannels() \!= tempBuffer.getNumChannels()  
        || buf.getNumSamples() \< numSamples)  
        buf.setSize (tempBuffer.getNumChannels(), numSamples, false, false, true);  
}

**The Fix:**  
You are already doing this properly for voiceScratch in SynthVoice::prepareToPlay. Move the layerScratch and layerAsyncScratch resizing logic there too.  
C++  
// In SynthVoice::prepareToPlay:  
layerScratch.setSize(outputChannels, samplesPerBlock);  
for(auto& buf : layerAsyncScratch) {  
    buf.setSize(outputChannels, samplesPerBlock);  
}

// In SynthVoice::renderNextBlock:  
// REMOVE the if() statements entirely.   
// Just clear the portion of the pre-allocated buffer you are about to use:  
layerScratch.clear(0, numSamples);   
// Then pass \`numSamples\` to your processing functions.

### **3\.** std::async **on the Audio Thread**

You are using std::async to parallelize your layer rendering.  
**The Problem:**  
C++  
layerFutures\[static\_cast\<size\_t\> (slot)\] \= std::async (std::launch::async, ...

std::async is not real-time safe. Depending on the compiler's standard library implementation, it allocates memory to store the shared state of the std::future, and it asks the OS scheduler to spin up or wake a thread, which involves kernel-level mutex locks. future.wait() will put the audio thread to sleep while it waits, destroying your real-time deadline.  
**The Fix:**  
For 4 synth layers, the overhead of dispatching standard threads often costs *more* CPU time than just rendering the 4 layers synchronously one after the other.  
I strongly recommend **removing** std::async **entirely** and just rendering your audible layers sequentially in a simple for loop. If your DSP code is optimized (especially using juce::dsp and SIMD), a modern CPU can easily calculate 4 layers of wavetable/sampler math simultaneously in serial without dropping frames.  
*If you absolutely must multi-thread*, you have to do it lock-free. You would need to use something like juce::ThreadPool instantiated globally, and use a lock-free queue to dispatch ThreadPoolJob objects, using a lightweight juce::WaitableEvent or atomic spinlock to wait for completion. But again, thread pooling is usually done at the *Voice* level (calculating 8 voices on 8 different threads), not at the *Layer* level within a single voice.

### **Summary**

Your DSP logic (Wavetables, Mod Matrix, Filters) looks fantastic. You've clearly put an incredible amount of work into the mathematics and the feature set.  
If you fix **Issue \#1** (caching your APVTS pointers to eliminate strings in processBlock), you will see an immediate, massive reduction in CPU usage. If you fix **Issue \#2** and **Issue \#3**, you will eliminate the audio glitches/dropouts.

# Opus 4.8

# **Softysn1 — Code Review**

**Date:** 2026-06-29 **Scope:** DSP / audio-thread real-time safety, architecture & maintainability, UI/editor code, performance hot paths. **Codebase:** \~25,800 lines across 205 C++ files (JUCE VST, multi-part synth with Lua scripting host).

---

## **Summary**

This is a well-architected codebase. Cross-thread state uses `std::atomic`, `processBlock` opens with `ScopedNoDenormals`, there is a smart "skip render when idle" fast path, the look-and-feel is torn down correctly, and there is a clean central parameter-ID registry (`Softysn1::Params`). The synth voice and DSP modules are clean — no allocations or locks in the voice render path.

The problems that matter are concentrated in one place: the per-block parameter synchronisation at the top of `Source/Core/PluginProcessor.cpp::processBlock`. Fixing that single area removes the only real-time-safety hazards and most of the per-block CPU overhead.

Severity legend: **High** \= can cause audible dropouts/glitches or undefined behaviour; **Medium** \= maintainability / scalability; **Low** \= polish.

---

## **High — real-time safety in `processBlock`**

### **1\. \~190 heap allocations per audio block from string-built parameter IDs**

The top of `processBlock` rebuilds `juce::String` parameter IDs every block to look parameters up:

* LFO custom-wave tables: 4 LFOs × 16 points (`PluginProcessor.cpp:284–294`)  
* Mod-matrix slots: 16 × 4 fields, e.g. `"MOD" + juce::String(i) + "_SRC"` (`:395–402`)  
* 12 macros (`:313–333`)  
* 16 drum steps \+ 16 step probabilities (`:611–617`)  
* 16 step-mod levels (`:652–656`)

Every `juce::String` concatenation allocates on the heap. That is roughly 190 allocations **per block**; at 48 kHz with 64-sample blocks (\~750 blocks/sec) it is well over 100,000 heap allocations per second on the audio thread. Allocation on the audio thread takes a lock inside the allocator and is the classic cause of intermittent dropouts and glitches under load.

**Fix:** cache `std::atomic<float>*` pointers once (constructor or `prepareToPlay`) into fixed-size arrays, then index them in the audio thread — no strings, no map lookups, no allocations. Sketch:

// header  
std::array\<std::atomic\<float\>\*, kNumModSlots \* 4\>  modSlotParams {};  
std::array\<std::atomic\<float\>\*, 4 \* kLfoCustomPoints\> lfoWaveParams {};  
// ... macros, drum steps, step-mod, etc.

// prepareToPlay (or a cacheParameterPointers() helper called once)  
for (int i \= 0; i \< kNumModSlots; \++i) {  
    const auto p \= "MOD" \+ juce::String(i);  
    modSlotParams\[i\*4+0\] \= apvts.getRawParameterValue(p \+ "\_SRC");  
    modSlotParams\[i\*4+1\] \= apvts.getRawParameterValue(p \+ "\_DEST");  
    modSlotParams\[i\*4+2\] \= apvts.getRawParameterValue(p \+ "\_AMT");  
    modSlotParams\[i\*4+3\] \= apvts.getRawParameterValue(p \+ "\_CURVE");  
}

// processBlock — no string building  
for (int i \= 0; i \< kNumModSlots; \++i) {  
    activePatch().modSlots\[i\].source.store(modSlotParams\[i\*4+0\]-\>load());  
    activePatch().modSlots\[i\].dest  .store(modSlotParams\[i\*4+1\]-\>load());  
    activePatch().modSlots\[i\].amount.store(modSlotParams\[i\*4+2\]-\>load());  
    activePatch().modSlots\[i\].curve .store(modSlotParams\[i\*4+3\]-\>load());  
}

This is the single highest-value change in the codebase.

### **2\. Two `juce::MidiBuffer` allocated per block**

The velocity-remap block (`:539–556`) and the MPE filter block (`:558–598`) each construct a fresh `juce::MidiBuffer` every block and then `swapWith` it. `MidiBuffer` owns a heap buffer, so this allocates on the audio thread.

**Fix:** make them reusable members and call `.clear()` at the start of each block instead of constructing new ones. After warm-up the internal storage stops reallocating.

// header  
juce::MidiBuffer midiScratchA, midiScratchB;  
// processBlock  
midiScratchA.clear();  
// ... fill midiScratchA ...  
midiMessages.swapWith(midiScratchA);

### **3\. File I/O on the audio thread**

`applyDeferredPresetIfReady()` is called first thing in `processBlock` (`:144`). When an FX preset is pending it constructs an `FxPresetManager` and calls `loadPreset(deferredFxPresetFile)` — a disk read plus XML parse inside the audio callback (`:1808–1817`). It only fires when the user loads a preset, but it can stall an entire block and cause an audible gap.

**Fix:** parse the preset on the message/worker thread into a ready-to-apply structure, hand it to the audio thread via an atomic pointer / lock-free FIFO, and let the audio thread only perform the (cheap, non-allocating) swap.

### **4\. The whole "copy every APVTS value into the matrix every block" pattern is O(hundreds) unconditionally**

Beyond the string-built lookups, there are \~88 additional `getRawParameterValue` hash-map lookups per block (`:147–466`). Each is a map lookup that runs whether or not the value changed. Combined with \#1 this is the bulk of the fixed per-block CPU cost.

**Fix:** the pointer-caching from \#1 covers correctness/allocation. To also cut the redundant work, consider driving the matrix from APVTS parameter listeners (write-on-change) instead of polling all parameters every block, or at minimum cache all the pointers and skip the map lookups. Most of these are GUI-rate parameters that do not need per-block polling.

---

## **Medium — architecture & maintainability**

### **5\. `PluginProcessor.cpp` is 1,820 lines**

About 260 lines are the mechanical per-block APVTS→matrix copy and \~500 lines are `createParameterLayout()`. Both are mechanical and would move cleanly into dedicated helpers (e.g. an `ApvtsCache` / `ParameterBlock` type and a `ParameterLayout` builder file). This shrinks the processor, isolates the hot path so it is reviewable on its own, and naturally pushes you toward the pointer-caching in \#1.

### **6\. Minor dead code (not the forwarding shims)**

The top-level `Source/PluginProcessor.h`, `Source/PresetManager.h`, `Source/PluginEditor.h`, `Source/VoiceParameterMatrix.h`, and `Source/UI/BioSynthLookAndFeel.h` are **intentional one-line forwarding includes**, not duplicates — leave them. Genuinely unreferenced files are only:

* `Source/UI/UIHelpers.h`  
* `Source/DSP/Multi/VoiceMatrixCopy.h`  
* `Source/DSP/Multi/LegacyPartBridge.h`  
* `Source/DSP/LayerStack.h`  
* `Source/SynthEditorContent.cpp` (481 lines; not in the CMake build)

Worth confirming and removing to reduce noise, but low impact.

---

## **Low — polish**

### **7\. `juce::Random::getSystemRandom()` in the audio thread**

Used at `:713` for the sample-and-hold fallback. `getSystemRandom()` returns a shared instance; prefer a per-processor member `juce::Random` to avoid any shared-state contention.

### **8\. Master-gain read/write inconsistency**

Master gain is written from the `MASTER_GAIN` parameter into `activePatch().masterGain` (`:161`) but read back from `multiPatch.masterGain` (`:694`) when applying gain. These are two different storage locations; verify they are intentionally kept in sync, otherwise the `MASTER_GAIN` knob and the applied gain can diverge.

---

## **What looks good (keep doing this)**

* `ScopedNoDenormals` at the top of `processBlock`.  
* All cross-thread synth state goes through `std::atomic`.  
* The `skipSynthRender` idle fast path avoids running the full graph when nothing is sounding.  
* Voice render path (`SynthVoice.cpp`) and DSP/effect modules are allocation- and lock-free.  
* UI: look-and-feel is set and cleared in pairs; no risky `MessageManager::callAsync([this]...)` captures spotted; timers are stopped.  
* Central `Softysn1::Params` ID registry instead of scattered string literals.

---

## **Suggested order of work**

1. **\#1 (cache parameter pointers)** — removes most allocations and most per-block CPU. Highest value.  
2. **\#2 (reusable MIDI scratch buffers)** — small, low-risk, removes the remaining per-block allocations.  
3. **\#3 (preset I/O off the audio thread)** — removes the last real-time hazard.  
4. **\#5 (split the processor)** — makes the above easier to maintain and review.  
5. **\#6–\#8** — cleanup when convenient.

All four high-priority items live in `Source/Core/PluginProcessor.{h,cpp}`, so they can be done in one focused pass.

