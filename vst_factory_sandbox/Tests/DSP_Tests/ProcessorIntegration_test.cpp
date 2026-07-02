#include <catch2/catch_test_macros.hpp>

#include "Core/PluginProcessor.h"
#include "Helpers/AllocationCounter.h"

TEST_CASE ("VstFactorySandboxAudioProcessor processBlock does not allocate", "[rt][allocation][integration]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    VstFactorySandboxAudioProcessor processor;
    processor.prepareToPlay (48000.0, 512);

    juce::AudioBuffer<float> buffer (2, 512);
    buffer.clear();
    juce::MidiBuffer midi;

    for (int block = 0; block < 4; ++block)
        processor.processBlock (buffer, midi);

    VSTFactory::Test::resetAllocationCount();
    VSTFactory::Test::setCountingEnabled (true);

    for (int block = 0; block < 8; ++block)
        processor.processBlock (buffer, midi);

    VSTFactory::Test::setCountingEnabled (false);
    REQUIRE (VSTFactory::Test::getAllocationCount() == 0);
}

TEST_CASE ("VstFactorySandboxAudioProcessor warm re-open does not crash", "[warm][integration]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    for (int instance = 0; instance < 3; ++instance)
    {
        auto processor = std::make_unique<VstFactorySandboxAudioProcessor>();
        processor->prepareToPlay (48000.0, 512);

        juce::AudioBuffer<float> buffer (2, 512);
        buffer.clear();
        juce::MidiBuffer midi;

        for (int block = 0; block < 2; ++block)
            processor->processBlock (buffer, midi);

        processor->releaseResources();
        processor.reset();
    }

    SUCCEED();
}
