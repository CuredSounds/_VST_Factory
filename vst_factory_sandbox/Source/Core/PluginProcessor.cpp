#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters.h"

VstFactorySandboxAudioProcessor::VstFactorySandboxAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", VSTFactory::Params::createParameterLayout())
{
    gainParameter = apvts.getRawParameterValue (VSTFactory::Params::gain);
    bypassParameter = apvts.getRawParameterValue (VSTFactory::Params::bypass);
}

VstFactorySandboxAudioProcessor::~VstFactorySandboxAudioProcessor() {}

const juce::String VstFactorySandboxAudioProcessor::getName() const { return "VST_Factory_Sandbox"; }
bool VstFactorySandboxAudioProcessor::acceptsMidi() const { return true; }
bool VstFactorySandboxAudioProcessor::producesMidi() const { return true; }
bool VstFactorySandboxAudioProcessor::isMidiEffect() const { return false; }
double VstFactorySandboxAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int VstFactorySandboxAudioProcessor::getNumPrograms() { return 1; }
int VstFactorySandboxAudioProcessor::getCurrentProgram() { return 0; }
void VstFactorySandboxAudioProcessor::setCurrentProgram (int index) { juce::ignoreUnused(index); }
const juce::String VstFactorySandboxAudioProcessor::getProgramName (int index) { juce::ignoreUnused(index); return {}; }
void VstFactorySandboxAudioProcessor::changeProgramName (int index, const juce::String& newName) { juce::ignoreUnused(index, newName); }

void VstFactorySandboxAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void VstFactorySandboxAudioProcessor::releaseResources() {}

bool VstFactorySandboxAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void VstFactorySandboxAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Safe parameter loading using atomic pointers
    const bool isBypassed = bypassParameter ? bypassParameter->load() > 0.5f : false;
    if (isBypassed)
        return;

    const float gainDb = gainParameter ? gainParameter->load() : 0.0f;
    const float gainLinear = juce::Decibels::decibelsToGain (gainDb);

    if (gainLinear != 1.0f)
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer (channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                channelData[sample] *= gainLinear;
            }
        }
    }
}

bool VstFactorySandboxAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* VstFactorySandboxAudioProcessor::createEditor()
{
    return new VstFactorySandboxAudioProcessorEditor (*this);
}

void VstFactorySandboxAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void VstFactorySandboxAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

// Creation entry point for JUCE plugins
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VstFactorySandboxAudioProcessor();
}
