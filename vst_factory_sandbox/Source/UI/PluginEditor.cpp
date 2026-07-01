#include "PluginEditor.h"
#include "../Core/Parameters.h"

VstFactorySandboxAudioProcessorEditor::VstFactorySandboxAudioProcessorEditor (VstFactorySandboxAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);

    gainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (gainSlider);

    bypassButton.setButtonText ("Bypass");
    addAndMakeVisible (bypassButton);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getAPVTS(), VSTFactory::Params::gain, gainSlider);

    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        audioProcessor.getAPVTS(), VSTFactory::Params::bypass, bypassButton);
}

VstFactorySandboxAudioProcessorEditor::~VstFactorySandboxAudioProcessorEditor() = default;

void VstFactorySandboxAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("VST Factory Sandbox", getLocalBounds().removeFromTop (40),
                      juce::Justification::centred, 1);
}

void VstFactorySandboxAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (40);

    gainSlider.setBounds (area.removeFromLeft (150).reduced (10));
    bypassButton.setBounds (area.removeFromLeft (100).reduced (10).withHeight (30)
                                               .withY (getHeight() / 2 - 15));
}
