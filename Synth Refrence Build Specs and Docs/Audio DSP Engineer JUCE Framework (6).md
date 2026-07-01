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

\*\*Your Task:\*\*  
Please acknowledge you understand the architectural goals, the Phase Plant/Multipass paradigms, and the real-time constraints. Once acknowledged, provide a brief bulleted plan for the new Phase. I fully acknowledge and understand the architectural goals, the Kilohearts-style paradigms, and the strict real-time constraints for this project.  
The transition to a Composite Design Pattern using AudioModule, routing containers, and lock-free matrices is the exact path required to evolve this from a static synthesizer into a dynamic sound design environment. The constraints regarding real-time safety (no allocations, no locks on the audio thread) and standardizing state management via juce::ValueTree are noted and will be strictly adhered to.

Begin work on the next phase below

Welcome to \*\*Phase 6: Dynamic UI Routing & Window Resizing\*\*.

This is where your JUCE frontend catches up to the modular DSP backend. We are going to rip out the hardcoded FX sliders and toggle buttons from \`Softysn1AudioProcessorEditor\`, make the window resizable to accommodate a growing rack, and implement JUCE's \`DragAndDropContainer\` to allow Kilohearts-style module reordering.

Here is the exact surgical plan to transform your UI into a dynamic modular rack.

\#\#\# 1\. Update the Editor Header (\`PluginEditor.h\`)

We need to turn the main editor into a \`DragAndDropContainer\` and delete all the hardcoded master FX sliders and buttons. We will replace them with a dynamic array of \`EffectSlotComponent\`s (which we will define in the next step).

\`\`\`cpp  
\#pragma once

\#include \<juce\_audio\_processors/juce\_audio\_processors.h\>  
\#include "PluginProcessor.h"

// \--- ADD THIS FOR THE DYNAMIC UI SLOTS \---  
class EffectSlotComponent;

class Softysn1AudioProcessorEditor : public juce::AudioProcessorEditor,  
                                     public juce::DragAndDropContainer // \--- ADD THIS \---  
{  
public:  
    Softysn1AudioProcessorEditor (Softysn1AudioProcessor&);  
    \~Softysn1AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;  
    void resized() override;  
      
    // \--- ADD THIS TO REFRESH UI WHEN ROUTING CHANGES \---  
    void buildRackUI();

private:  
    // ... \[KEEP OSCILLATOR, FILTER, ENV, LFO, UNISON SLIDERS/COMBOBOXES\] ...

    // \--- DELETE ALL MASTER FX BUTTONS, SLIDERS, AND ATTACHMENTS \---  
    // Remove fxDriveOnButton, fxChorusRateSlider, fxDriveAmtAttachment, etc.

    // \--- ADD THE DYNAMIC RACK COMPONENTS \---  
    juce::OwnedArray\<EffectSlotComponent\> rackSlots;

    // Preset controls  
    juce::Label presetNameLabel;  
    juce::TextButton prevPresetButton { "\<" };  
    juce::TextButton nextPresetButton { "\>" };  
    juce::TextButton savePresetButton { "Save" };  
    juce::TextButton loadPresetButton { "Load" };

    Softysn1AudioProcessor& audioProcessor;

    JUCE\_DECLARE\_NON\_COPYABLE\_WITH\_LEAK\_DETECTOR (Softysn1AudioProcessorEditor)  
};

\`\`\`

\#\#\# 2\. Create the Drag-and-Drop Target (\`EffectSlotComponent.h\`)

This class represents a single module lane in your UI. It acts as both a drag source (when the user clicks and drags it) and a drop target (when the user hovers another module over it). You can place this definition at the top of your \`PluginEditor.cpp\` file or in a new header.

\`\`\`cpp  
class EffectSlotComponent : public juce::Component,  
                            public juce::DragAndDropTarget  
{  
public:  
    EffectSlotComponent(Softysn1AudioProcessorEditor& ed, Softysn1AudioProcessor& proc, int index)  
        : editor(ed), processor(proc), slotIndex(index)  
    {  
        // Get the module name from the DSP chain  
        if (auto\* fx \= processor.getMasterEffectChain().getEffect(slotIndex))  
            moduleName \= fx-\>getName();  
        else  
            moduleName \= "Empty Slot";  
    }

    void paint (juce::Graphics& g) override  
    {  
        auto bounds \= getLocalBounds().toFloat();  
          
        // Draw highlight if being hovered over by a dragged module  
        if (isHoveredForDrop)  
            g.setColour(juce::Colour(0xff4a9eff).withAlpha(0.3f)); // Kilohearts blue tint  
        else  
            g.setColour(juce::Colour(0xff2a2a2a));  
              
        g.fillRoundedRectangle(bounds, 4.0f);  
          
        g.setColour(juce::Colours::white);  
        g.drawText(moduleName, getLocalBounds(), juce::Justification::centred);  
    }

    // \--- DRAG SOURCE IMPLEMENTATION \---  
    void mouseDrag(const juce::MouseEvent& e) override  
    {  
        if (e.mods.isLeftButtonDown() && e.mouseWasDraggedSinceMouseDown())  
        {  
            if (auto\* dragContainer \= juce::DragAndDropContainer::findParentDragContainerFor(this))  
            {  
                auto dragImage \= createComponentSnapshot(getLocalBounds(), false, 0.5f);  
                // The description string tells the container WHICH slot is moving  
                dragContainer-\>startDragging("RackSlot\_" \+ juce::String(slotIndex), this, dragImage);  
            }  
        }  
    }

    // \--- DRAG TARGET IMPLEMENTATION \---  
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override  
    {  
        return dragSourceDetails.description.toString().startsWith("RackSlot\_");  
    }

    void itemDragEnter(const SourceDetails&) override { isHoveredForDrop \= true; repaint(); }  
    void itemDragExit(const SourceDetails&) override  { isHoveredForDrop \= false; repaint(); }

    void itemDropped(const SourceDetails& dragSourceDetails) override  
    {  
        isHoveredForDrop \= false;  
          
        // Parse the source index from the string (e.g., "RackSlot\_2" \-\> 2\)  
        int sourceIndex \= dragSourceDetails.description.toString().substring(9).getIntValue();  
          
        if (sourceIndex \!= slotIndex)  
        {  
            // \--- CRITICAL THREAD SAFETY \---  
            // Freeze the audio thread, move the pointers, unfreeze, then redraw UI  
            processor.suspendProcessing(true);  
            processor.getMasterEffectChain().moveEffect(sourceIndex, slotIndex);  
            processor.suspendProcessing(false);  
              
            editor.buildRackUI();  
        }  
    }

private:  
    Softysn1AudioProcessorEditor& editor;  
    Softysn1AudioProcessor& processor;  
    int slotIndex;  
    juce::String moduleName;  
    bool isHoveredForDrop \= false;  
};

\`\`\`

\#\#\# 3\. Editor Constructor & Resizing (\`PluginEditor.cpp\`)

Now we implement the window resizing and call \`buildRackUI()\` to dynamically draw the backend DSP chain.

\`\`\`cpp  
Softysn1AudioProcessorEditor::Softysn1AudioProcessorEditor (Softysn1AudioProcessor& p)  
    : AudioProcessorEditor (\&p), audioProcessor (p)  
{  
    // ... \[KEEP EXISTING UI SETUP FOR SYNTH CONTROLS\] ...

    // \--- DELETE ALL THE HARDCODED FX UI INITIALIZATION \---

    // \--- ENABLE RESIZING \---  
    setResizable(true, true);  
    setResizeLimits(760, 740, 1400, 1000); // Set minimum and maximum window bounds

    // Build the dynamic UI rack based on whatever is loaded in the backend  
    buildRackUI();  
      
    // Set initial size  
    setSize (760, 740);  
}

void Softysn1AudioProcessorEditor::buildRackUI()  
{  
    rackSlots.clear(); // Clear existing UI slots  
      
    auto& chain \= audioProcessor.getMasterEffectChain();  
    int numEffects \= chain.getNumEffects();  
      
    for (int i \= 0; i \< numEffects; \++i)  
    {  
        auto\* slotComponent \= rackSlots.add(new EffectSlotComponent(\*this, audioProcessor, i));  
        addAndMakeVisible(slotComponent);  
    }  
      
    resized(); // Force layout recalculation  
}

\`\`\`

\#\#\# 4\. Layout Calculation (\`PluginEditor.cpp\` \-\> \`resized()\`)

Finally, update your \`resized()\` method to loop through the \`rackSlots\` array and place them dynamically, rather than relying on hardcoded \`setBounds\` for individual sliders.

\`\`\`cpp  
void Softysn1AudioProcessorEditor::resized()  
{  
    // ... \[KEEP YOUR SYNTH ROW LAYOUTS EXACTLY AS THEY ARE\] ...  
      
    auto area \= getLocalBounds().reduced (16);  
    area.removeFromTop (32); // Synth Row 1  
    area.removeFromTop (44); // Synth Row 2  
    area.removeFromTop (100); // Synth Row 3  
    area.removeFromTop (100); // Synth Row 4  
    area.removeFromTop (100); // Synth Row 5

    // \--- PHASE 6: DYNAMIC RACK LAYOUT \---  
    area.removeFromTop(20); // Gap before FX  
    auto fxRackArea \= area.removeFromTop(80); // Set height for the module blocks

    if (\!rackSlots.isEmpty())  
    {  
        // Divide the available width by the number of active modules  
        const int slotWidth \= fxRackArea.getWidth() / rackSlots.size();  
          
        for (int i \= 0; i \< rackSlots.size(); \++i)  
        {  
            rackSlots\[i\]-\>setBounds(fxRackArea.removeFromLeft(slotWidth).reduced(4));  
        }  
    }

    // Preset bar at bottom  
    auto presetBar \= getLocalBounds().removeFromBottom (36).reduced (8, 4);  
    // ... \[keep preset bar bounds\] ...  
}

\`\`\`  
