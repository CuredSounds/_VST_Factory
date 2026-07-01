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

