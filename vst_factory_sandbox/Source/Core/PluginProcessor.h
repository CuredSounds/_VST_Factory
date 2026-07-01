#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../DSP/EffectModules/GainProcessor/GainProcessor.h"

class VstFactorySandboxAudioProcessor : public juce::AudioProcessor
{
public:
    VstFactorySandboxAudioProcessor();
    ~VstFactorySandboxAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    const VSTFactory::DSP::GainProcessor& getGainProcessor() const { return gainProcessor; }

private:
    juce::AudioProcessorValueTreeState apvts;
    VSTFactory::DSP::GainProcessor gainProcessor;

    std::atomic<float>* gainParameter = nullptr;
    std::atomic<float>* bypassParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VstFactorySandboxAudioProcessor)
};
