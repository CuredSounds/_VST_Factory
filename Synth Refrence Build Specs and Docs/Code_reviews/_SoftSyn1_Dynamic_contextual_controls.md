# ChatGPT 5.5

Yes — and your instinct is right. The UI should become **contextual**, not a museum wall of knobs. A synth should feel like opening a drawer: pick FM6, FM6 tools appear; pick sampler, sampler tools appear; pick wavetable, wavetable tools appear. Right now the controls exist, but they are still scattered like parts on a bench.

## **1\. Dynamic oscillator controls: yes, implement it**

Your current `OscColumnHub` already has the start of this idea. It shows/hides the wavetable editor when `OSC_TYPE == 4`, shows FM6 controls when `OSC_TYPE == 9`, and hides/shows the 2-op FM controls otherwise.

The problem is that the layout is still fixed into columns: OSC A has wavetable controls, OSC B has sync/ring controls, OSC C has FM/FM6 controls.  That means the UI is saying “this is where the knobs live,” instead of “this is the engine you selected, here are its tools.”

### **Better structure**

Create a reusable `OscSlotPanel`:

class OscSlotPanel : public juce::Component,  
                     private juce::AudioProcessorValueTreeState::Listener  
{  
public:  
    enum class Slot { A, B, C };

    OscSlotPanel (Softysn1AudioProcessor&, Slot);

private:  
    void rebuildForOscType (int oscType);  
    void showOnlyRelevantControls();  
    void resized() override;

    juce::ComboBox oscTypeSelector;  
    std::unique\_ptr\<juce::Component\> visualArea;  
    std::unique\_ptr\<juce::Component\> controlArea;  
};

Then each oscillator zone becomes:

\[ waveform / scope / algorithm visual \]  
\[ oscillator type selector            \]  
\[ contextual controls for this engine \]

That is exactly what you described.

### **Use descriptors, not hardcoded UI spaghetti**

Make a table that describes each oscillator engine:

struct OscEngineDescriptor  
{  
    int oscType;  
    juce::String name;  
    juce::String visualMode;  
    std::vector\<const char\*\> parameterIds;  
};

Example:

static const OscEngineDescriptor fm6Descriptor  
{  
    9,  
    "FM6",  
    "FM6AlgorithmView",  
    {  
        "FM6\_ALGO",  
        "FM6\_INDEX",  
        "FM6\_FEEDBACK",  
        "FM6\_RATIO2",  
        "FM6\_RATIO3",  
        "FM6\_RATIO4",  
        "FM6\_RATIO5",  
        "FM6\_RATIO6"  
    }  
};

Then the UI builds itself from the selected engine. This is cleaner than manually calling `setVisible()` on ten random controls forever.

### **Practical first implementation**

Do not rewrite the whole UI yet. Refactor `OscColumnHub` in phases:

1. Move current OSC A visual/controls into an `OscEnginePanel`.  
2. Keep your existing attachments.  
3. Replace the static FM/FM6 control placement with a contextual `juce::Viewport` or `juce::FlexBox`.  
4. On oscillator type change, call `rebuildForOscType()` and `resized()`.

You already attach controls for wavetable, 2-op FM, FM6, sync, unison, etc.  The job is mostly **layout intelligence**, not new DSP.

---

## **2\. The waveform/scope area should become clickable and expandable**

Yes, clicking the waveform display should open a larger visual page. That is a strong UI/UX upgrade.

You already have `WavetableEditor` and `OscShapeView` inside `OscColumnHub`.  The timer updates the wavetable position and swaps between osc shape view and wavetable editor.

### **Add a “Visual Matrix” window**

Make a new component:

class ModulationVisualMatrix : public juce::DocumentWindow  
{  
public:  
    ModulationVisualMatrix (Softysn1AudioProcessor&);

private:  
    WaveformOverviewPanel waveforms;  
    LfoOverviewPanel lfos;  
    ModRoutingOverlayPanel routes;  
};

When the user clicks the waveform:

oscShapeView.onClick \= \[this\]  
{  
    openVisualMatrix();  
};

wtEditor.onClick \= \[this\]  
{  
    openVisualMatrix();  
};

The window should show:

OSC A waveform  
OSC B waveform  
OSC C waveform  
LFO 1 shape \+ destination text  
LFO 2 shape \+ destination text  
LFO 3 shape \+ destination text  
LFO 4 shape \+ destination text  
MSEG shapes  
Mod matrix source → destination overlays  
Current wavetable frame / position

This is one of those features that makes a complex synth feel less like a cockpit and more like a map.

### **Avoid a detached window at first**

Detached plugin windows can be host-hostile. Some DAWs get weird. Start with a modal internal overlay:

addAndMakeVisible(visualMatrixOverlay);  
visualMatrixOverlay.toFront(false);

Then later add true floating-window support.

---

## **3\.**

## **`+`**

## **/**

## **`-`**

## **resizing: yes, easy win**

Your editor already handles keyboard shortcuts: Cmd+Z, Cmd+Shift+Z, F1, and Cmd+L.  It already has `UI_SCALE`, timer-based scale updates, resize limits, and `setScaleFactor()`.

So add:

if (key \== juce::KeyPress ('+', 0, 0\) || key \== juce::KeyPress ('=', 0, 0))  
{  
    auto\* p \= audioProcessor.getAPVTS().getParameter ("UI\_SCALE");  
    const float current \= audioProcessor.getAPVTS().getRawParameterValue ("UI\_SCALE")-\>load();  
    const float next \= juce::jlimit (0.75f, 1.5f, current \+ 0.05f);

    p-\>beginChangeGesture();  
    p-\>setValueNotifyingHost (p-\>convertTo0to1 (next));  
    p-\>endChangeGesture();  
    return true;  
}

if (key \== juce::KeyPress ('-', 0, 0))  
{  
    auto\* p \= audioProcessor.getAPVTS().getParameter ("UI\_SCALE");  
    const float current \= audioProcessor.getAPVTS().getRawParameterValue ("UI\_SCALE")-\>load();  
    const float next \= juce::jlimit (0.75f, 1.5f, current \- 0.05f);

    p-\>beginChangeGesture();  
    p-\>setValueNotifyingHost (p-\>convertTo0to1 (next));  
    p-\>endChangeGesture();  
    return true;  
}

Also add `Cmd+0` to reset to 1.0.

Small feature, big feel.

---

## **4\. Sampler: yes, it needs a real waveform editor**

Right now the sampler panel has sample load, filename label, loop selector, root note, start, end, loop start, and loop end.  That is a basic parameter strip, not yet a real sampler.

You want something closer to the Ableton/Bitwig workflow:

\[ waveform viewer \]  
\[ start marker | end marker \]  
\[ loop start | loop end \]  
\[ slice markers \]  
\[ normalize | gain | reverse | fade in/out \]  
\[ root note | key zone | velocity zone \]

### **Add**

### **`SampleWaveformEditor`**

class SampleWaveformEditor : public juce::Component  
{  
public:  
    void setSampleData (std::shared\_ptr\<const SampleData\> sample);  
    void setMarkers (float start, float end, float loopStart, float loopEnd);  
    void setSlices (std::vector\<float\> normalisedSlicePositions);

    std::function\<void(float)\> onStartChanged;  
    std::function\<void(float)\> onEndChanged;  
    std::function\<void(float)\> onLoopStartChanged;  
    std::function\<void(float)\> onLoopEndChanged;  
    std::function\<void(int, float)\> onSliceMoved;

private:  
    std::vector\<float\> minPeaks;  
    std::vector\<float\> maxPeaks;  
};

Do **not** draw every sample. Precompute a peak cache:

struct WaveformPeak  
{  
    float min \= 0.0f;  
    float max \= 0.0f;  
};

std::vector\<WaveformPeak\> buildPeakCache (const juce::AudioBuffer\<float\>& buffer,  
                                          int targetWidth);

This keeps drawing cheap.

### **Add sampler gain and normalize**

Add parameters:

SMPL\_GAIN\_DB      *// \-24 dB to \+24 dB*  
SMPL\_NORMALIZE\_ON *// bool, optional*  
SMPL\_FADE\_IN\_MS  
SMPL\_FADE\_OUT\_MS  
SMPL\_REVERSE

I would make **Normalize** non-destructive at first:

float computeNormalizeGainDb (const juce::AudioBuffer\<float\>& buffer)  
{  
    float peak \= 0.0f;

    for (int ch \= 0; ch \< buffer.getNumChannels(); \++ch)  
        peak \= juce::jmax (peak, buffer.getMagnitude (ch, 0, buffer.getNumSamples()));

    if (peak \<= 0.00001f)  
        return 0.0f;

    return juce::Decibels::gainToDecibels (0.98f / peak);  
}

Then store it as `SMPL_GAIN_DB`. No destructive editing needed.

### **Slices**

You already have `estimateSliceCountForZone()` and sample-zone support in the processor.   Build on that.

Add:

struct SampleSlice  
{  
    float startNorm \= 0.0f;  
    float endNorm \= 1.0f;  
    int midiNote \= 60;  
};

Then save slices into plugin state.

Add UI buttons:

Detect Slices  
Clear Slices  
Slice Sensitivity  
Play Slice Mode: Off / One-Shot / Gate / Slice Map

Minimum viable sampler upgrade:

1. Draw waveform.  
2. Drag start/end.  
3. Drag loop start/end.  
4. Normalize button.  
5. Gain knob.  
6. Detect slices.  
7. Click slice to audition.

That alone makes it feel like an instrument instead of a file loader.

---

## **5\. Performance improvements: most likely wins**

### **Biggest CPU/UI suspects**

Your editor timer runs at 12 Hz and updates UI scale/theme/preset state and repaints the header.  That is okay, but several child components also use timers, including `OscColumnHub`.

`OscColumnHub` polls APVTS and UI modulation data at 12 Hz.  Fine for now, but as visual panels grow, switch to dirty flags and lower rates.

### **Audio-thread improvements**

Your `processBlock()` currently copies a large number of APVTS values into `activePatch()` every block.  That works, but it is brute force.

Better:

struct ParameterCache  
{  
    std::atomic\<bool\> dirty { true };  
    CachedPatchValues values;  
};

Use APVTS listeners to mark dirty, then only refresh blocks when needed.

### **Avoid building dynamic containers in the audio path**

The arpeggiator builds a `std::vector` pattern inside `process()`, and `process()` can run from the audio callback.   Pre-reserve or use fixed arrays:

std::array\<int, 128\> pattern {};  
int patternSize \= 0;

### **Avoid system random in audio callback**

Inside `processBlock()`, this line uses `juce::Random::getSystemRandom()` as part of sample-hold fallback behavior.  I would replace that with a processor-owned lightweight RNG seeded in `prepareToPlay()`.

float fastNoise()  
{  
    rngState \= rngState \* 1664525u \+ 1013904223u;  
    return ((rngState \>\> 8\) / 16777215.0f) \* 2.0f \- 1.0f;  
}

Old-school, ugly, fast. Sometimes the farm tractor beats the spaceship.

### **Make visual drawing cached**

For waveform, wavetable, LFO, and modulation overlays:

audio data changes → rebuild cached path/image  
component resized → rebuild cached path/image  
timer tick → move playhead only

Do not redraw all paths from scratch every paint if nothing changed.

---

## **6\. Ableton crash on exit: likely causes and fixes**

This is the most urgent bug. Host exit crashes are usually dangling pointers, active timers, async callbacks, static pointers, or audio/editor lifetime mismatch.

### **Suspect 1: static wavetable preview pointer**

`WavetablePreview` holds a static raw pointer:

static const WavetableData\* imported;

and `setImportedSource()` stores it directly.

The processor points that to `userWavetable` when loading a wavetable.

If the processor is destroyed but a timer, UI paint, or static preview path still queries `WavetablePreview`, that pointer can dangle. That is a prime suspect.

Fix:

Softysn1AudioProcessor::\~Softysn1AudioProcessor()  
{  
    WavetablePreview::setImportedSource (nullptr);  
}

Also make `WavetablePreview::imported` safer long-term:

static std::atomic\<const WavetableData\*\> imported;

or remove the static pointer entirely and pass preview data through the processor/editor.

### **Suspect 2: async file chooser captures**

### **`this`**

Both wavetable and sample loading use async file choosers whose callbacks capture `this`.

If Ableton closes the plugin while a chooser/callback exists, that can call into a dead editor/component.

Fix pattern:

juce::Component::SafePointer\<OscColumnHub\> safeThis (this);

chooser-\>launchAsync (...,  
    \[safeThis, chooser\] (const juce::FileChooser& fc)  
    {  
        if (safeThis \== nullptr)  
            return;

        auto& self \= \*safeThis;  
        *// safe use*  
    });

Do that everywhere an async lambda captures `this`.

### **Suspect 3: editor destructor should aggressively detach callbacks**

The editor destructor currently does:

stopTimer();  
setLookAndFeel (nullptr);

Good, but add more cleanup:

Softysn1AudioProcessorEditor::\~Softysn1AudioProcessorEditor()  
{  
    stopTimer();

    cockpitUI.setOnPresetChanged (nullptr);

    presetPrevButton.onClick \= nullptr;  
    presetNextButton.onClick \= nullptr;  
    presetSaveButton.onClick \= nullptr;  
    presetStarButton.onClick \= nullptr;  
    snapshotAButton.onClick \= nullptr;  
    snapshotBButton.onClick \= nullptr;  
    randomizeButton.onClick \= nullptr;  
    midiCaptureButton.onClick \= nullptr;

    tooltipWindow.reset();

    setLookAndFeel (nullptr);  
}

Also add destructors to timer-owning child components and call `stopTimer()` explicitly.

### **Crash-debug checklist**

Build a Debug VST3 and test this exact flow:

1\. Open Ableton.  
2\. Load only Softysn1.  
3\. Do not open plugin UI. Quit.  
4\. Open Ableton.  
5\. Load Softysn1. Open UI. Quit.  
6\. Open Ableton.  
7\. Load wavetable/sample. Quit.  
8\. Repeat with plugin window open.

If it only crashes after opening UI: editor/timer/async pointer.

If it crashes without opening UI: processor destructor, static pointer, audio-thread shutdown, or plugin wrapper issue.

---

## **7\. Suggested implementation order**

### **Phase 1 — crash and stability**

1. Clear `WavetablePreview::imported` in processor destructor.  
2. Replace async chooser lambdas with `SafePointer`.  
3. Stop timers in all timer-owning panels.  
4. Detach editor callbacks in destructor.  
5. Test Ableton quit repeatedly.

### **Phase 2 — contextual oscillator UI**

1. Refactor current `OscColumnHub` into contextual groups.  
2. Move FM6 controls directly under OSC A selector when FM6 is selected.  
3. Put waveform/scope/algorithm visual above the selector.  
4. Add `+`, `-`, and `Cmd+0` UI zoom shortcuts.  
5. Save UI scale in APVTS.

### **Phase 3 — visual matrix**

1. Click waveform opens overlay page.  
2. Show OSC waveforms, LFOs, MSEGs, and routing text.  
3. Add modulation overlays.  
4. Keep it internal overlay first, floating window later.

### **Phase 4 — sampler upgrade**

1. Add `SampleWaveformEditor`.  
2. Build waveform peak cache.  
3. Add draggable start/end/loop markers.  
4. Add gain and normalize.  
5. Add slice detection and slice markers.  
6. Add slice audition and MIDI slice mapping.

---

## **Blunt design call**

Yes, change it. Static synth UIs are tolerable when the synth is simple. Your synth is not simple. With FM6, wavetable, sampler, granular, physical modeling, modulation, FX, and multi-parts, a static knob field becomes a swamp.

Make the UI behave like a good shop wall: when you grab the router, the router bits appear; when you grab the oscilloscope, the probes are right there. No treasure hunt. No “where did I put that 10mm socket?” energy.

# Gemini Pro

These are fantastic UI/UX and workflow ideas. Moving from a static "knob-per-function" interface to a dynamic, context-aware interface is exactly what elevates a plugin from feeling like a basic tech demo to a professional, modern instrument (like Serum, Phase Plant, or Ableton's native devices).  
Here is a breakdown of how to architect and implement these features within your JUCE codebase.

### **1\. Dynamic Oscillator Controls (Context-Aware UI)**

Having static UI space taken up by FM controls when you are using a Wavetable is definitely a waste of space.  
**How to implement in JUCE:**  
Instead of laying out all controls statically, group your controls into distinct sub-components (e.g., class FM6Controls : public juce::Component, class WavetableControls : public juce::Component).

1. **Overlay them in the same space:** Add them all to your OscColumnHub or LayersPanel, and set their bounds to the exact same area under the selection menu.  
2. **Listen for the Osc Type parameter:** Have your GUI class inherit from juce::AudioProcessorValueTreeState::Listener.  
3. **Attach the listener:** apvts.addParameterListener("layerAosc", this);  
4. **React and switch visibility:**  
5. C++

void parameterChanged (const juce::String& parameterID, float newValue) override  
{  
    if (parameterID \== "layerAosc")  
    {  
        // GUI updates must happen on the message thread\!  
        juce::MessageManager::callAsync(\[this, newValue\]() {  
            int oscType \= static\_cast\<int\>(newValue);  
            fm6Controls.setVisible(oscType \== 3); // Example index  
            wavetableControls.setVisible(oscType \== 1);  
            samplerControls.setVisible(oscType \== 5);

            // Trigger a layout recalculation if heights vary  
            resized();   
        });  
    }  
}

6.   
7. 

### **2\. Resizing the Master Scope via Keystrokes**

**How to implement in JUCE:**

1. Ensure your main editor or the scope component can receive key events by calling setWantsKeyboardFocus(true) in its constructor.  
2. Override the keyPressed method.  
3. Adjust a member variable (like scopeHeightMultiplier) and call resized().  
4. C++

bool keyPressed (const juce::KeyPress& key) override  
{  
    if (key.getKeyCode() \== juce::KeyPress::plusKey || key.getKeyCode() \== '=') {  
        scopeHeightMultiplier \+= 0.1f;  
        resized(); // Tell the parent layout to give this component more room  
        return true;   
    }  
    if (key.getKeyCode() \== juce::KeyPress::minusKey || key.getKeyCode() \== '-') {  
        scopeHeightMultiplier \-= 0.1f;  
        resized();  
        return true;  
    }  
    return false;  
}

5.   
6. 

### **3\. Ableton Live Crashing on Exit**

When a DAW crashes specifically on *exit* or *plugin removal*, it is almost entirely guaranteed to be an **Order of Destruction** issue or a **Thread Tearing** issue.

* **The** std::async **Issue:** As mentioned in the previous review, std::async is dangerous in audio. If Ableton tries to delete your plugin while an async task is still running in the background, the memory it is trying to access is suddenly deleted, causing an Access Violation (Segfault) and taking Ableton down with it. Moving away from std::async to synchronous processing or a proper lock-free thread pool will likely fix this.  
* **Dangling Pointers in UI:** Your PluginEditor is destroyed *before* your PluginProcessor. If any of your UI components (like scopes or analyzers) are holding a raw pointer or reference to a DSP object, and a background thread tries to update the UI while the UI is being destroyed, it will crash.  
* **APVTS Listeners:** Ensure you are calling apvts.removeParameterListener(paramID, this) in the destructors of your GUI components if you use the listener approach mentioned in step 1\.

### **4\. Waveform Click \-\> Mod/LFO Overlay Page**

This is a brilliant UX idea. You want an "X-Ray" view of your modulations.  
**How to implement in JUCE:**

1. In your WaveformDisplay component, override the mouse click:  
2. C++

void mouseUp (const juce::MouseEvent& e) override  
{  
    if (e.mods.isLeftButtonDown()) {  
        // Tell the main editor to open the overlay  
        if (auto\* mainEditor \= findParentComponentOfClass\<Softysn1AudioProcessorEditor\>())  
            mainEditor-\>showModulationOverlay();  
    }  
}

3.   
4.   
5. In your Main Editor, create an OverlayComponent that covers the whole screen.  
6. C++

void Softysn1AudioProcessorEditor::showModulationOverlay()  
{  
    modOverlay.setVisible(true);  
    modOverlay.setBounds(getLocalBounds());   
    modOverlay.toFront(true); // Bring it above all other panels  
}

7.   
8.   
9. The modOverlay can draw lines connecting sources to destinations (similar to Bitwig's inspector) with a slightly transparent dark background (g.fillAll(juce::Colours::black.withAlpha(0.85f));).

### **5\. Sampler Improvements (Ableton/Bitwig Style)**

To make a truly capable sampler, you need a dedicated interactive viewer.

* **Waveform Viewer & Trimming:** Create a SampleDisplayComponent. Pass it the sampleBuffer. In paint(), draw the peaks (using buffer.getMagnitude(ch, start, numSamples) to draw min/max lines for performance).  
* To implement trimming, create two child components (DragHandle class). Constrain their X-positions to the bounds of the waveform. In their mouseDrag callback, map their X-coordinate (0 to width) to a normalized float (0.0f to 1.0f) and update your APVTS parameters (smplStart and smplEnd).  
* **Normalize:**  
* Do this non-destructively so you don't alter the original file data.  
  1. Scan the loaded sampleBuffer to find the absolute maximum sample value (e.g., 0.5f).  
  2. Calculate the normalization multiplier: float normGain \= 1.0f / maxSample; (e.g., 2.0f).  
  3. Add a juce::AudioParameterFloat for "Sampler Gain".  
  4. In your SamplerModule DSP code, when reading a sample, multiply it: output \= rawSample \* normGain \* samplerGainParam;.  
* **Slices:**  
* Since you already have estimateSliceCountForZone in your processor, you can return a std::vector\<int\> of sample indices where transients occur. In your SampleDisplayComponent, loop through this vector, map the sample indices to X-pixels, and use g.drawVerticalLine() to draw the slice markers over the waveform.

# Opus 4.8

# **Softysn1 — Feature Implementation Plan**

**Date:** 2026-06-29 **Covers:** contextual per-engine osc controls, resizable scopes, CPU reduction, the Ableton exit crash, a modulation-overview window, and a full sampler editor (Ableton/Bitwig-style).

Each section gives the problem, where it lives in the current code, a recommended approach, and the concrete steps. Ordered by suggested priority.

---

## **1\. Ableton exit crash (do this first — it's a real bug)**

### **Most likely cause**

`Source/UI/BioSynthFonts.cpp:9–11` holds three embedded typefaces in **namespace-scope (global) storage**:

namespace {  
    juce::Typeface::Ptr displayFace;  
    juce::Typeface::Ptr monoRegularFace;  
    juce::Typeface::Ptr monoBoldFace;  
}

These are reference-counted JUCE objects that are never released until C++ static destruction, which on plugin unload runs **after** Ableton has begun tearing down JUCE's graphics/font subsystem. Releasing a `Typeface` at that point touches already-destroyed global state and crashes. This is one of the best-documented "crashes only on exit" patterns in JUCE plugins, and it matches your symptom exactly (only the synth loaded, crash on quit, not during use).

### **Fix (preferred): tie font lifetime to the editor via `SharedResourcePointer`**

Make the typefaces members of a small holder owned through `juce::SharedResourcePointer`, so they're created when the first editor opens and destroyed when the last editor closes — always while JUCE is still alive.

// BioSynthFonts.h  
struct FontBank {  
    juce::Typeface::Ptr display, monoRegular, monoBold;  
    FontBank();                 // load from BinaryData here  
};

// usage: hold one in the editor  
juce::SharedResourcePointer\<FontBank\> fonts;   // member of the editor

Have `displayFont()` / `monoFont()` take the bank (or read it from a `SharedResourcePointer` they construct locally — the resource stays alive as long as the editor's copy does).

### **Fix (minimal): explicitly release on editor close**

If you'd rather not refactor, add `BioSynthFonts::releaseFonts()` that nulls the three `Ptr`s, and call it from `Softysn1AudioProcessorEditor::~Softysn1AudioProcessorEditor()` (alongside the existing `setLookAndFeel(nullptr)`). That guarantees release happens on the message thread while JUCE is alive. The risk: if another editor instance is open it would drop the cache early — `SharedResourcePointer` avoids that, so prefer the first option.

### **Verify**

The real proof is a stack trace. In Ableton, run a Debug build of the plugin and check the crash backtrace points into `Typeface`/`CoreText`/`juce::Font` teardown. Also worth ruling out as secondary suspects:

* `juce::dsp::Convolution` in `ConvolutionReverbModule` spawns a background thread; confirm the module (and so the Convolution) is destroyed before the processor finishes tearing down. It normally self-joins, but verify there's no dangling reference to processor state.  
* The `WeakReference<Softysn1AudioProcessor>` \+ `callAsync` path in `loadFxPreset` — fine as long as `JUCE_DECLARE_WEAK_REFERENCEABLE` is present and a queued lambda can't outlive the message manager. Low risk, but check.

---

## **2\. Contextual per-engine oscillator controls**

### **Current state**

`OscColumnHub` already declares every control for every engine as a member and toggles a subset with `setVisible()` in a 12 Hz `timerCallback` (`OscColumnHub.cpp:153–177`) — but only for Wavetable / 2-op FM / FM6. The other engines in your 15-type list (Sampler, Granular, Karplus, Modal, Additive, Spectral, String, Vowel, Organ — `OscTypeChoices.h`) have their controls scattered across other panels. That's exactly the "static controls in random spots" problem.

### **Recommended approach: an engine-editor swap zone**

Treat the area directly under the OSC A selector as a single **engine editor host** that shows exactly one child component — the editor for the currently selected engine.

1. Define one small `Component` subclass per engine (`WavetableOscEditor`, `Fm6OscEditor`, `GranularOscEditor`, `SamplerOscEditor`, `KarplusOscEditor`, …). Each owns only its own sliders/combos and their APVTS attachments, and implements its own `resized()`. Several already exist as loose control groups elsewhere — moving them into dedicated editors is mostly cut-and-paste.

In the host, keep them in `std::array<std::unique_ptr<juce::Component>, kNumOscTypes>` (or a map). Show one at a time:  
 void setEngine(int oscType) {    if (oscType \== current) return;    if (auto\* prev \= editors\[current\].get()) prev-\>setVisible(false);    current \= oscType;    auto\* next \= editors\[current\].get();    addAndMakeVisible(\*next);    next-\>setBounds(engineZone);   // engineZone computed in resized()}

2.   
3. **Drive it from the parameter, not the timer.** Use an `AudioProcessorValueTreeState::Listener` (or `ParameterAttachment`) on `OSC_TYPE` so the swap happens exactly when the engine changes, on the message thread. This also removes per-frame `setVisible` churn. Keep a timer only for the live scope animation.  
4. Above the zone, host one **scope view** that itself switches representation by engine (wavetable stack for WT, operator/algorithm diagram for FM6, sample thumbnail for Sampler, harmonic bars for Additive, etc.). You already have building blocks in `WaveformDisplay.h` (`CycleScope`, `WavetableStackView`, `OscShapeView`).

### **Why this is worth it**

It localizes each engine's UI to one file, kills the "where did that knob go" problem, and makes adding a new engine a self-contained task (new editor \+ register it). It also shrinks `OscColumnHub` substantially.

### **Migration path (low-risk, incremental)**

* Step 1: extract the **existing** WT / FM / FM6 controls into three editor components and host-swap them — behavior identical, but now structured.  
* Step 2: move one scattered engine at a time (start with Granular or Sampler) into its own editor.  
* Step 3: delete the now-empty control groups from the other panels.

---

## **3\. Resizable scopes / using the empty master-scope area**

Two separate asks here.

### **a) Resize panels with \+/- keystrokes**

The editor already handles keys in `Softysn1AudioProcessorEditor::keyPressed` (`PluginEditor.cpp:252`). Add a "focused scope" concept:

* Track which scope is under the mouse or last clicked (`activeScopeId`).  
* On `'='`/`'+'` and `'-'`, adjust a stored height/zoom factor for that scope and call `resized()`.  
* Persist the factor as a non-automatable APVTS value or in the editor state so it survives reopen.

A cleaner alternative that DAW users expect: make the scope area a draggable splitter (JUCE `StretchableLayoutManager` \+ `StretchableLayoutResizerBar`). Keystrokes can still nudge the same layout value. I'd offer both — splitter for discoverability, keys for speed.

### **b) Fill the large empty master-scope region**

That space is ideal for the per-engine scope from §2 and/or the modulation-overview from §5. Rather than a single static master oscilloscope, make it a **switchable visual bay**: tabs or a small selector toggling between {Master output scope, Active engine scope, Mod overview, Spectrum}. Reuses the scope components you already have and gives the empty area a purpose.

---

## **4\. CPU reduction**

The biggest wins overlap with the prior code review (see `Claude_softsyn_review_2026-06-29.md`): the per-block APVTS→matrix copy in `processBlock` does \~190 heap allocations and \~88 map lookups **every block, unconditionally**. Fixing that (cache `std::atomic<float>*` pointers; reuse MIDI scratch buffers) is the single largest CPU \+ glitch win. Beyond that:

* **Make parameter→matrix sync change-driven.** Most of those parameters are GUI-rate. Drive the matrix from APVTS parameter listeners (write-on-change) instead of polling all of them per block. Eliminates almost all of the fixed per-block cost.  
* **Control-rate modulation.** You already have `LFO_CONTROL_RATE` and `ECO_MODE`. Ensure LFOs/MSEGs/slow mod sources update at block or sub-block rate (e.g. every 16–32 samples) rather than per-sample when in non-audio-rate modes. Confirm the granular `maxGrains` cap and voice tiers actually gate work.  
* **Skip silent parts/effects.** `processBlock` already has `skipSynthRender` and `partAudible` — extend the idea to per-effect bypass: an effect at default/zero mix shouldn't run its inner loop. Check the convolution reverb and oversampled effects especially; oversampling is often the dominant cost.  
* **Voice stealing / max polyphony.** Make sure idle voices fully short-circuit (the voice path looked clean, but verify `renderNextBlock` returns early when the amp envelope is finished rather than processing zeros through filters).  
* **Denormals.** `ScopedNoDenormals` is set globally — good; just confirm feedback paths (delays, combs, Karplus) flush-to-zero so they don't drop into denormal territory between notes.

Recommend profiling with a single sustained patch per engine to attribute cost; the telemetry you already compute (`getCpuPercent`) plus a sampling profiler (Instruments / VTune) will point at the real hot engines quickly.

---

## **5\. Modulation-overview window (click a scope → see all sources/destinations)**

### **Approach**

A modal-less `juce::DocumentWindow` (or an in-editor overlay like the existing `midiLearnOverlay`) that renders:

* All LFOs / MSEGs / envelopes as live animated curves (reuse `LfoShapeView`, `MsegGraphEditor`, `AdsrGraphEditor`).  
* The mod-matrix routing as a source→destination list or node graph, read from the 16 mod slots (`activePatch().modSlots[i]` — source/dest/amount/curve, already atomics).  
* Text overlays naming each route, using your existing `getModSourceNames()` / `getModDestNames()`.

### **Wiring**

* Add a click handler on the scope components (`mouseDown`) that calls an editor callback to open/toggle the overview.  
* The overview reads matrix state at \~30 Hz via a timer (all atomics, so no locking) and repaints.  
* For "show overlays of which source hits which destination," highlight a route on hover and draw a connector line to the corresponding control if it's on-screen.

Start read-only (visualization) — it's genuinely useful on its own — then optionally allow editing routes from the same view later.

---

## **6\. Sampler editor (Ableton/Bitwig-style)**

### **Current state**

* DSP (`SamplerModule.h`) supports start/end, loop start/end, 3 loop modes, root note, linear interpolation, 4 key zones. It has **no gain, no normalize, no slice playback**.  
* UI (`SampleMapPanel`) is sliders \+ a "SLICE SCAN" button and a slice-count label; **no waveform display, no trim/loop handles, no slice markers**.  
* The processor owns the audio (`sampleZoneBuffers`, `juce::AudioBuffer<float>`) and exposes `estimateSliceCountForZone`.

### **a) Waveform viewer with trim \+ slices**

Use `juce::AudioThumbnail` built from the zone buffer (it's designed exactly for this and is cheap to repaint):

* A `SampleWaveformView` component that draws the thumbnail, plus draggable handles for start/end and loop start/end, mapped to the existing `SMPL_START` / `SMPL_END` / `SMPL_LOOP_START` / `SMPL_LOOP_END` params via `ParameterAttachment`.  
* Draw slice markers as vertical lines at the detected slice positions; clicking a marker can audition or set a play range.  
* Drag handles write to the params; the DSP already reads them, so playback follows immediately.

### **b) Normalize and gain**

These need new state:

* **Gain:** add an `AudioParameterFloat` `SMPL_GAIN` (e.g. −24…+24 dB) and apply it in `SamplerModule::getNextSample` (multiply `out` by a linear gain alongside `velGain`). Trivial DSP change, fully automatable.  
* **Normalize:** a non-realtime, message-thread operation. Scan the loaded zone buffer for peak, compute the gain that brings peak to 0 dBFS (or a target), and either (i) apply it destructively to the stored buffer, or (ii) store it as a per-zone offset combined with `SMPL_GAIN`. Prefer non-destructive (store a `normalizeGain` per zone) so it's reversible. Add a "Normalize" button per zone in the panel.

### **c) Slices**

You already estimate slice counts. To make slices playable like Ableton's Simpler "Slice" mode:

* Store slice boundary positions per zone (transient-detected or evenly divided).  
* Add a slice playback mode where incoming MIDI notes map chromatically to slices (note → slice index → start/end range).  
* Visualize boundaries in the waveform view; allow drag to adjust, click to add/remove.

### **Matching Ableton/Bitwig feel**

The expected interactions: drag waveform edges to trim, shift-drag for loop region, double-click to zoom to selection, scroll to zoom, snap handles to zero-crossings (avoids clicks), and a clear visual distinction between play range (start/end) and loop range. Zero-crossing snap is a small helper that searches outward from the dragged sample for a sign change — cheap and a big quality boost.

### **Suggested order**

1. `SMPL_GAIN` param \+ apply in DSP (smallest, immediately useful).  
2. `AudioThumbnail` waveform view with draggable start/end/loop handles.  
3. Normalize button (non-destructive per-zone gain).  
4. Slice markers → slice playback mode.

---

## **Recommended overall sequence**

1. **Exit crash** (§1) — small, removes a shipping-blocker.  
2. **CPU / per-block sync fix** (§4 \+ the prior review) — biggest stability/perf win, also makes everything else smoother.  
3. **Sampler gain \+ waveform/trim view** (§6a–b) — high user value, self-contained.  
4. **Contextual engine controls** (§2) — the largest UI refactor; do it incrementally.  
5. **Switchable visual bay \+ resize** (§3) and **mod overview** (§5) — build on the components from §2.

All of these are additive and can land independently; none requires a parameter-state break except adding `SMPL_GAIN` (append it to the layout so existing presets still load).

