#include "PluginProcessor.h"
#include "../UI/PluginEditor.h"
#include "Parameters.h"
#include "../DSP/EffectModules/GainProcessor/GainProcessor.h"

VstFactorySandboxAudioProcessor::VstFactorySandboxAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", VSTFactory::Params::createParameterLayout())
{
    gainParameter = apvts.getRawParameterValue (VSTFactory::Params::gain);
    bypassParameter = apvts.getRawParameterValue (VSTFactory::Params::bypass);
}

VstFactorySandboxAudioProcessor::~VstFactorySandboxAudioProcessor() = default;

const juce::String VstFactorySandboxAudioProcessor::getName() const { return "VST_Factory_Sandbox"; }
bool VstFactorySandboxAudioProcessor::acceptsMidi() const { return true; }
bool VstFactorySandboxAudioProcessor::producesMidi() const { return true; }
bool VstFactorySandboxAudioProcessor::isMidiEffect() const { return false; }
double VstFactorySandboxAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int VstFactorySandboxAudioProcessor::getNumPrograms() { return 1; }
int VstFactorySandboxAudioProcessor::getCurrentProgram() { return 0; }
void VstFactorySandboxAudioProcessor::setCurrentProgram (int index) { juce::ignoreUnused (index); }
const juce::String VstFactorySandboxAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}
void VstFactorySandboxAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void VstFactorySandboxAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
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

void VstFactorySandboxAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                      juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const bool isBypassed = bypassParameter != nullptr && bypassParameter->load() > 0.5f;
    const float gainDb = gainParameter != nullptr ? gainParameter->load() : 0.0f;
    const float gainLinear = juce::Decibels::decibelsToGain (gainDb);

    gainProcessor.setBypassed (isBypassed);
    gainProcessor.setGainLinear (gainLinear);

    float* channelPointers[2] = {};
    const int numChannels = juce::jmin (totalNumInputChannels, 2);
    for (int ch = 0; ch < numChannels; ++ch)
        channelPointers[ch] = buffer.getWritePointer (ch);

    gainProcessor.processBlock (channelPointers, numChannels, buffer.getNumSamples());
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

    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VstFactorySandboxAudioProcessor();
}
