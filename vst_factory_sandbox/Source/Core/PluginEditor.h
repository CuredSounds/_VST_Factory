#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class VstFactorySandboxAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    VstFactorySandboxAudioProcessorEditor (VstFactorySandboxAudioProcessor&);
    ~VstFactorySandboxAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    VstFactorySandboxAudioProcessor& audioProcessor;

    juce::Slider gainSlider;
    juce::ToggleButton bypassButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VstFactorySandboxAudioProcessorEditor)
};
