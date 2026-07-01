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

Phase 3 is where we finally strip the hardcoded generation logic out of your `SynthVoice` and wrap it into a self-contained, Snap Heap-style node.

Currently, `SynthVoice` is doing the heavy lifting of managing 7 unison oscillators, a sub-oscillator, stereo spreading math, and a wavetable generator. In a true modular architecture, the Voice should simply tell a `GeneratorModule` what note to play, and the module handles its own internal DSP.

By passing the `VoiceParameterMatrix` directly into this new module, it can read the unison detune, sub level, and oversampling settings on its own, without bothering the `SynthVoice`.

Here is the exact surgery to execute Phase 3\.

### 1\. Create the Base Interface: `GeneratorModule.h`

Create this new file to define the polymorphic interface for all future sound generators (Samplers, Granular engines, Noise modules).

\#pragma once

\#include \<JuceHeader.h\>

\#include "VoiceParameterMatrix.h"

//==============================================================================

/\*\*

 \* Base interface for all audio generators.

 \* Unlike EffectModules which process existing audio, GeneratorModules 

 \* create audio from scratch based on frequency/MIDI input.

 \*/

class GeneratorModule

{

public:

    virtual \~GeneratorModule() \= default;

    virtual void prepare (double sampleRate, int maxBlockSize, int numChannels) \= 0;

    virtual void reset() \= 0;

    

    // Sets the fundamental pitch for the block

    virtual void setBaseFrequency (float hz) \= 0;

    

    // Fills the buffer with generated audio

    virtual void process (juce::AudioBuffer\<float\>& buffer) \= 0;

};

### 2\. Create the Node: `OscillatorModule.h`

Create this file to encapsulate your analog shapes and wavetable logic. Notice how all the unison and sub-oscillator variables have been moved here from `SynthVoice`.

\#pragma once

\#include \<JuceHeader.h\>

\#include "GeneratorModule.h"

\#include "WavetableOscillator.h"

class OscillatorModule : public GeneratorModule

{

public:

    explicit OscillatorModule (VoiceParameterMatrix& matrix);

    \~OscillatorModule() override \= default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels) override;

    void reset() override;

    void setBaseFrequency (float hz) override;

    void process (juce::AudioBuffer\<float\>& buffer) override;

private:

    void updateConfiguration();

    void updateUnisonFrequencies();

    static int oversamplingChoiceToFactor (int choice);

    VoiceParameterMatrix& parameterMatrix;

    std::array\<juce::dsp::Oscillator\<float\>, 7\> unisonOscs;

    juce::dsp::Oscillator\<float\> subOsc;

    WavetableOscillator wavetableOsc;

    

    juce::AudioBuffer\<float\> unisonScratch;

    juce::Random random;

    float baseFrequencyHz \= 440.0f;

    int lastOscType \= \-1;

    int currentOversamplingFactor \= 1;

    

    // Local state to track changes

    int numUnisonVoices \= 1;

    float unisonDetune \= 0.0f;

    float unisonSpread \= 0.0f;

    float subLevel \= 0.0f;

    int subOctave \= 0;

    std::array\<float, 7\> unisonFrequencies {};

};

### 3\. Implement the Node: `OscillatorModule.cpp`

This file takes over all the complex rendering math. `SynthVoice` no longer has to worry about panning or unison math.

\#include "OscillatorModule.h"

OscillatorModule::OscillatorModule (VoiceParameterMatrix& matrix) 

    : parameterMatrix (matrix)

{

    updateConfiguration();

}

void OscillatorModule::prepare (double sampleRate, int maxBlockSize, int numChannels)

{

    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize \= static\_cast\<uint32\_t\>(maxBlockSize);

    spec.sampleRate \= sampleRate;

    spec.numChannels \= static\_cast\<uint32\_t\>(numChannels);

    for (auto& o : unisonOscs) o.prepare (spec);

    

    subOsc.prepare (spec);

    subOsc.initialise (\[\] (float x) { return std::sin (x); });

    

    unisonScratch.setSize (1, maxBlockSize);

    

    wavetableOsc.prepare (sampleRate);

    wavetableOsc.setOversamplingFactor (currentOversamplingFactor);

    wavetableOsc.setFrequency (baseFrequencyHz);

}

void OscillatorModule::reset()

{

    wavetableOsc.reset();

}

void OscillatorModule::setBaseFrequency (float hz)

{

    baseFrequencyHz \= juce::jmax (0.001f, hz);

    updateConfiguration();

    updateUnisonFrequencies();

}

void OscillatorModule::updateConfiguration()

{

    int oscType \= static\_cast\<int\>(parameterMatrix.oscType.load());

    float wtPos \= parameterMatrix.wavetablePosition.load();

    int osChoice \= static\_cast\<int\>(parameterMatrix.wavetableOversampling.load());

    wavetableOsc.setPosition (wtPos);

    

    int newFactor \= oversamplingChoiceToFactor(osChoice);

    if (newFactor \!= currentOversamplingFactor)

    {

        currentOversamplingFactor \= newFactor;

        wavetableOsc.setOversamplingFactor (currentOversamplingFactor);

        wavetableOsc.setFrequency (baseFrequencyHz);

    }

    if (oscType \!= lastOscType)

    {

        lastOscType \= oscType;

        switch (oscType)

        {

            case 0: for (auto& o : unisonOscs) o.initialise (\[\] (float x) { return std::sin (x); }); break;

            case 1: for (auto& o : unisonOscs) o.initialise (\[\] (float x) { return x / juce::MathConstants\<float\>::pi; }); break;

            case 2: for (auto& o : unisonOscs) o.initialise (\[\] (float x) { return x \< 0.0f ? \-1.0f : 1.0f; }); break;

            case 3: for (auto& o : unisonOscs) o.initialise (\[this\] (float) { return random.nextFloat() \* 2.0f \- 1.0f; }); break;

            case 4: break; // Wavetable

        }

    }

    numUnisonVoices \= juce::jlimit (1, 7, static\_cast\<int\>(parameterMatrix.unisonVoices.load()));

    unisonDetune \= parameterMatrix.unisonDetune.load();

    unisonSpread \= parameterMatrix.unisonSpread.load();

    subLevel \= parameterMatrix.subLevel.load();

    subOctave \= static\_cast\<int\>(parameterMatrix.subOctave.load());

}

void OscillatorModule::updateUnisonFrequencies()

{

    const float detuneCents \= unisonDetune \* 50.0f; 

    for (int v \= 0; v \< 7; \++v)

    {

        const float semitones \= (numUnisonVoices \> 1\)

            ? (static\_cast\<float\> (v) / static\_cast\<float\> (numUnisonVoices \- 1\) \- 0.5f) \* detuneCents / 100.0f

            : 0.0f;

        unisonFrequencies\[v\] \= baseFrequencyHz \* std::pow (2.0f, semitones);

        unisonOscs\[v\].setFrequency (juce::jmax (0.001f, unisonFrequencies\[v\]));

    }

    

    const float subFreq \= (subOctave \== 1\) ? baseFrequencyHz \* 0.25f : baseFrequencyHz \* 0.5f;

    subOsc.setFrequency (juce::jmax (0.001f, subFreq));

    wavetableOsc.setFrequency (baseFrequencyHz);

}

int OscillatorModule::oversamplingChoiceToFactor (int choice)

{

    switch (choice) { case 0: return 1; case 1: return 2; case 2: return 4; case 3: return 8; default: return 1; }

}

void OscillatorModule::process (juce::AudioBuffer\<float\>& buffer)

{

    const int numSamples \= buffer.getNumSamples();

    

    if (lastOscType \== 4\) // Wavetable

    {

        for (int sample \= 0; sample \< numSamples; \++sample)

        {

            float acc \= 0.0f;

            for (int os \= 0; os \< currentOversamplingFactor; \++os)

                acc \+= wavetableOsc.processSample();

                

            const float value \= acc / static\_cast\<float\> (currentOversamplingFactor);

            for (int ch \= 0; ch \< buffer.getNumChannels(); \++ch)

                buffer.addSample (ch, sample, value);

        }

    }

    else // Analog Unison

    {

        if (numUnisonVoices \== 1\)

        {

            juce::dsp::AudioBlock\<float\> oscBlock (buffer);

            juce::dsp::ProcessContextReplacing\<float\> oscCtx (oscBlock);

            unisonOscs\[0\].process (oscCtx);

        }

        else

        {

            for (int v \= 0; v \< numUnisonVoices; \++v)

            {

                juce::dsp::AudioBlock\<float\> monoBlock (unisonScratch.getArrayOfWritePointers(), 1, numSamples);

                juce::dsp::ProcessContextReplacing\<float\> monoCtx (monoBlock);

                unisonOscs\[v\].process (monoCtx);

                const float pan \= (static\_cast\<float\> (v) / static\_cast\<float\> (numUnisonVoices \- 1\) \- 0.5f) \* 2.0f \* unisonSpread;

                const float cp  \= juce::jlimit (-1.0f, 1.0f, pan);

                const float sc  \= 1.0f / static\_cast\<float\> (numUnisonVoices);

                const float lGain \= (1.0f \- cp) \* sc;

                const float rGain \= (1.0f \+ cp) \* sc;

                for (int s \= 0; s \< numSamples; \++s)

                {

                    const float samp \= unisonScratch.getSample (0, s);

                    buffer.addSample (0, s, samp \* lGain);

                    if (buffer.getNumChannels() \> 1\)

                        buffer.addSample (1, s, samp \* rGain);

                }

            }

        }

        // Sub Oscillator

        if (subLevel \> 0.001f)

        {

            juce::dsp::AudioBlock\<float\> subBlock (unisonScratch.getArrayOfWritePointers(), 1, numSamples);

            juce::dsp::ProcessContextReplacing\<float\> subCtx (subBlock);

            subOsc.process (subCtx);

            for (int ch \= 0; ch \< buffer.getNumChannels(); \++ch)

                for (int s \= 0; s \< numSamples; \++s)

                    buffer.addSample (ch, s, unisonScratch.getSample (0, s) \* subLevel);

        }

    }

}

### 4\. Strip Down `SynthVoice.h`

Now look at how clean the polyphonic voice becomes. We delete all the oscillator variables and replace them with a single `OscillatorModule`.

\#pragma once

\#include \<juce\_audio\_basics/juce\_audio\_basics.h\>

\#include \<juce\_dsp/juce\_dsp.h\>

\#include "Lfo.h"

\#include "VoiceParameterMatrix.h"

\#include "OscillatorModule.h" // Add this

class SynthVoice : public juce::SynthesiserVoice

{

public:

    explicit SynthVoice (VoiceParameterMatrix& matrix);

    

    // ... \[keep override declarations\] ...

    void pullParametersFromMatrix();

    void prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels);

    void renderNextBlock (juce::AudioBuffer\<float\>& outputBuffer, int startSample, int numSamples) override;

private:

    VoiceParameterMatrix& parameterMatrix;

    

    // \--- THIS IS ALL THAT REMAINS FOR GENERATION \---

    OscillatorModule primaryOscillator;

    // Filter & Env state

    juce::dsp::StateVariableTPTFilter\<float\> filter;

    juce::dsp::LadderFilter\<float\> ladderFilter;

    int activeFilterType \= 0;

    juce::dsp::Gain\<float\> gain;

    juce::ADSR adsr;

    juce::ADSR::Parameters adsrParams;

    float noteVelocity \= 1.0f;

    float baseFrequencyHz \= 440.0f;

    

    juce::ADSR filterEnv;

    juce::ADSR::Parameters filterEnvParams;

    float filterEnvAmount \= 0.0f;

    float baseCutoffHz \= 18000.0f;

    juce::SmoothedValue\<float, juce::ValueSmoothingTypes::Linear\> smoothedCutoff;

    

    Lfo lfo1;

    int lfo1Dest \= 0;

    float lfo1Depth \= 0.0f;

    bool isPrepared \= false;

};

### 5\. Finalize `SynthVoice.cpp`

Update the `SynthVoice` implementation to use the new module.

**Constructor & `prepareToPlay`:**

SynthVoice::SynthVoice(VoiceParameterMatrix& matrix) 

    : parameterMatrix(matrix), primaryOscillator(matrix) // Initialize the module

{

}

void SynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound\*, int)

{

    baseFrequencyHz \= static\_cast\<float\> (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));

    primaryOscillator.reset();

    noteVelocity \= velocity;

    adsr.noteOn();

    filterEnv.noteOn();

    lfo1.reset();

}

void SynthVoice::prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels)

{

    // ... \[keep juce::dsp::ProcessSpec spec setup\] ...

    primaryOscillator.prepare(sampleRate, samplesPerBlock, outputChannels);

    // ... \[keep filter and ADSR setups\] ...

}

**`pullParametersFromMatrix()`:** *Delete* all the `updateOscillator`, `unisonVoices`, and `subOctave` variable grabs. The `OscillatorModule` reads those itself now. Just keep the Filter, Envelope, and Master Gain parameter grabs.

**`renderNextBlock`:** Delete the massive `if (lastOscType == 4)` block that handled audio generation, and replace it with two lines:

    // LFO Pitch calculation

    float currentPitchHz \= baseFrequencyHz;

    if (lfo1Dest \== 2\)

    {

        const float pitchMult \= std::pow (2.0f, lfoValue \* lfo1Depth \* (2.0f / 12.0f));

        currentPitchHz \*= pitchMult;

    }

    

    // 1\. Tell the module what frequency to play

    primaryOscillator.setBaseFrequency(currentPitchHz);

    // 2\. Generate the raw audio into our tempBuffer

    primaryOscillator.process(tempBuffer);

    // 3\. Modulate and Filter...

    // ... \[keep your existing Filter, Gain, and ADSR processing code here\] ...

### The Result

You have successfully separated the *Signal Generation* from the *Voice Container*. If you want to add a second oscillator or a noise generator later, you just instantiate another `OscillatorModule` in `SynthVoice.h`, call `.process()` on it, and it will automatically tap into the UI matrix to get its settings.

We are one step away from having a `ParallelContainer` inside the voice that can dynamically load FX or Generators. Ready to verify compilation before we tackle Kilohearts-style LFO decoupling?  
