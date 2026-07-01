# Guardrails & Real-Time Safety Catalog

Guardrails are the Factory Model's "safety sensors" — hard constraints baked into the agent's
static context (`AGENTS.md`) so they're never re-litigated per prompt. This doc has two parts:
the rule list (short, goes in every `AGENTS.md`) and the anti-pattern catalog (long, this is
the reference material an agent or reviewer pulls up when diagnosing a slow or crashing plugin).

The anti-pattern catalog below is not hypothetical. Every entry is a real bug found in the
Softsyn1 codebase, independently confirmed by **five separate AI code reviews — Claude (twice),
ChatGPT, Gemini, and Kimi — all dated 2026-06-29**, reviewing the same branch. That convergence
is the strongest evidence available that these are exactly the right failure modes to guardrail
against mechanically rather than rely on a single reviewer to catch.

## A. The rule list (put this in AGENTS.md)

1. No memory allocation on the audio thread: no `new`/`malloc`, no `std::vector` resize, no
   `juce::String` construction/concatenation, no STL map insertion.
2. No locking, file I/O, console output, or blocking calls on the audio thread.
3. No `std::async`, `std::thread`, or future-waiting on the audio thread.
4. Pre-resolve every parameter lookup (ID → value/pointer) on parameter change; the audio thread
   only dereferences pre-cached pointers or reads atomics.
5. Block-size and sample-rate independence everywhere in the DSP layer.
6. All cross-thread state (UI ↔ audio, file-load ↔ audio) goes through atomics or lock-free
   SPSC structures — never a raw shared pointer/reference with no ownership contract.
7. Any object that owns a platform resource with non-deterministic static destruction order
   (fonts, GL contexts, native handles) gets an explicit, ordered teardown call from the plugin's
   shutdown path — never left to static destructors alone.
8. Embedded scripting engines (Lua, JS) run sandboxed — no filesystem or network access unless
   explicitly granted and reviewed.
9. Async callbacks/lambdas that capture `this` must either own a lifetime guarantee (the object
   cannot be destroyed while the callback is pending) or capture a weak/safe reference instead.

## B. Anti-pattern catalog (real bugs, what to check for, how it was caught)

### 1. String-built parameter IDs inside `processBlock` (the big one)

**What happened:** Parameter lookups for macros, mod-matrix slots, LFO custom waveforms, and
drum steps were resolved by concatenating `juce::String`s to build a key, then looking that key
up in a map — every single audio block. This produced roughly **190 heap allocations per
block** plus ~88 additional map lookups, entirely avoidable.

**Why every reviewer caught it independently:** it's the textbook violation of "pre-calculate
when parameters change, not inside the sample loop" — the single most emphasized rule in the
Day_1_v3 source material, and the most common way agentic code generation quietly reintroduces
non-RT-safe patterns because the generated code *looks* fine and compiles cleanly.

**Guardrail:** Rule #4 above. **Detection:** a CPU/allocation profiler run as a CI gate (see
`04_TESTING_AND_QUALITY_GATES.md`) that fails the build if `processBlock` allocates. Don't rely
on code review alone to catch this class of bug — it reads as ordinary, well-formatted code.

### 2. `std::async` per voice/layer on the audio thread

**What happened:** A per-voice-layer pattern launched `std::async` tasks from inside (or
adjacent to) the audio callback. `std::async` may allocate, may block on `get()`/destruction,
and gives no real-time scheduling guarantee — exactly what Rule #3 forbids.

**Guardrail:** Rule #3. Any "do this work and use the result later" pattern in DSP code needs a
lock-free queue or a pre-computed/cached result, never a future.

### 3. Two `MidiBuffer` allocations per block

**What happened:** MIDI event buffers were being constructed fresh per block instead of reused.

**Guardrail:** Rule #1. Reserve/reuse buffers in `prepareToPlay`, never construct containers
inside `processBlock`.

### 4. Static `Typeface::Ptr` teardown-order crash risk

**What happened:** Global static font objects (`BioSynthFonts.cpp`) had no defined teardown
order relative to other statics, risking a crash on host/app exit if destruction order didn't
cooperate.

**Guardrail:** Rule #7. Give resource-owning statics an explicit shutdown function called from a
known point in the plugin lifecycle, instead of trusting C++ static destruction order across
translation units.

### 5. File-chooser async lambda capturing unsafe `this`

**What happened:** An async file-chooser callback captured `this` directly; if the editor or
processor were destroyed while the dialog was open, the callback would fire on a dangling
pointer.

**Guardrail:** Rule #9. Use `juce::Component::SafePointer`, a weak reference, or an explicit
"am I still alive" flag for any UI-thread async callback that touches plugin state.

### 6. Sample/wavetable buffer mutation racing the audio thread

**What happened:** Wavetable or sample data buffers were being mutated (e.g. during import or
re-resampling) while the audio thread could simultaneously be reading from them, with no
synchronization.

**Guardrail:** Rule #6. Buffer swaps for sample/wavetable data should use a double-buffer +
atomic-pointer-swap pattern: build the new buffer off the audio thread, then atomically publish
a pointer to it. Never mutate a buffer in place that the audio thread may be reading.

### 7. Unsandboxed Lua scripting host

**What happened:** An embedded Lua host for automation/scripting had no restriction on
filesystem or system access.

**Guardrail:** Rule #8. Any embedded script engine gets an explicit allow-list of capabilities;
default-deny for file and network access.

### 8. Tail length and per-voice effect chain cost (earlier, already-fixed P0 round — keep as regression tests)

These predate the 2026-06-29 review round and were already fixed once (per Softsyn1's
`issues.md`/`P0.md`), but are worth keeping as permanent regression tests rather than assuming
"fixed" means "can't recur":

- `getTailLengthSeconds()` returning 0, chopping reverb/delay tails on note-off.
- Per-voice `EffectChain` allocation instead of a shared, pre-sized chain.
- `tempBuffer.setSize()` called inside `renderNextBlock` (an allocation in a hot path).

## C. How to use this catalog with an agent

When briefing an agent to write or review DSP code, point it at this file in addition to
`AGENTS.md`. When an agent's generated code needs review, check it against section B as a
checklist before checking against general code-quality concerns — these are the failure modes
most specific to, and most damaging in, real-time audio code, and they're exactly the kind of
"looks right, passes a casual read, fails under load" issue that the Day_1_v3 framework warns
generic code review will miss without automated tests behind it.
