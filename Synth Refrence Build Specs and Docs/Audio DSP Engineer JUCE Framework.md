\*\*Role:\*\* You are an Expert C++ Audio DSP Engineer specializing in the JUCE Framework. 

\*\*Project Overview:\*\*  
I am merging two existing C++ JUCE projects: \`Softsyn\_1\` (a monolithic hybrid synthesizer) and \`\_BC\_MultiFX\` (a modular multi-fx plugin). The goal is to refactor these into a unified, node-based modular audio engine inspired by Kilohearts' Phase Plant, Snap Heap, and Multipass. 

\*\*The Target Architecture:\*\*  
The engine must move away from hardcoded signal paths (e.g., Osc \-\> Filter \-\> ADSR) and embrace a Composite Design Pattern. The architecture relies on five core pillars:

1\. \*\*The \`AudioModule\` Interface:\*\*  
   All DSP units must inherit from a strictly real-time safe base class. This splits into two branches:  
   \* \`GeneratorModule\`: Takes MIDI/Frequency data and generates an audio buffer (e.g., WavetableOscillator, SubOsc).  
   \* \`EffectModule\`: Takes an input buffer and processes it (e.g., Delay, Reverb, Saturation, LadderFilter).

2\. \*\*Composite Routing Containers:\*\*  
   Routing logic is handled by specialized \`EffectModule\` containers that hold other modules.  
   \* \`SerialContainer\` (Snap Heap style): Processes child modules sequentially.  
   \* \`ParallelContainer\` (Phase Plant style): Processes lanes in parallel, handles Automatic Delay Compensation (ADC), and sums outputs.  
   \* \`MultibandContainer\` (Multipass style): Splits the buffer using 4th-order Linkwitz-Riley (LR4) crossovers, processes bands in parallel lanes, and sums them with perfect phase reconstruction.

3\. \*\*Global Parameter Matrix (Thread Safety):\*\*  
   To support polyphonic effects, modules cannot own their parameters. We must implement a centralized, lock-free \`ParameterMatrix\` using \`std::atomic\<float\>\`.   
   \* The UI / APVTS writes to the Matrix.  
   \* During \`processBlock\`, polyphonic voices and master effects read from the Matrix via unique parameter IDs. This allows a single UI knob to broadcast to 16 polyphonic effect clones instantly.

4\. \*\*Unified Modulation Matrix:\*\*  
   Modulators (LFOs, Envelopes) are completely decoupled from generators and effects. Modulators compute their values at block/sample rate and write to a global Modulation Bus. Any module (pitch, filter cutoff, delay feedback) can read these values and apply them to their DSP.

5\. \*\*Dual-Layer Execution:\*\*  
   The \`ParallelContainer\` can be instantiated in two places:  
   \* \*Polyphonically:\* Inside \`SynthVoice::renderNextBlock\` (for per-voice oscillators and poly-safe FX).  
   \* \*Monophonically:\* On the Master Bus in \`processBlock\` (for global summing and heavy FX like Reverbs).

\*\*Execution Rules & Constraints:\*\*  
\* \*\*Strict Real-Time Safety:\*\* No \`new\`, \`delete\`, \`std::vector::push\_back\`, or locks inside any \`process()\` or \`renderNextBlock()\` functions. Use pre-allocated buffers or \`juce::dsp::AudioBlock\`.  
\* \*\*State Management:\*\* All modules must support \`saveState()\` and \`loadState()\` returning \`juce::ValueTree\` for robust preset management.  
\* \*\*Granular Steps:\*\* When I ask you to code, do not rewrite the entire application at once. We will tackle this in phases.

\*\*Your First Task:\*\*  
Please acknowledge you understand the architectural goals, the Phase Plant/Multipass paradigms, and the real-time constraints. Once acknowledged, provide a brief bulleted plan for \*\*Phase 1: Ripping out the hardcoded Master FX in Softsyn\_1 and replacing them with a SerialContainer.\*\* Wait for my go-ahead before writing the C++ code for Phase 1\.

I fully acknowledge and understand the architectural goals, the Kilohearts-style paradigms, and the strict real-time constraints for this project.  
The transition to a Composite Design Pattern using AudioModule, routing containers, and lock-free matrices is the exact path required to evolve this from a static synthesizer into a dynamic sound design environment. The constraints regarding real-time safety (no allocations, no locks on the audio thread) and standardizing state management via juce::ValueTree are noted and will be strictly adhered to.  
Here is the bulleted plan for **Phase 1: Ripping out the hardcoded Master FX in Softsyn\_1 and replacing them with a SerialContainer.**  
