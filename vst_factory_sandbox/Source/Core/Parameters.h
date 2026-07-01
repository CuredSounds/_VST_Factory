#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace VSTFactory
{
    namespace Params
    {
        // Define parameter string IDs canonically here
        inline constexpr auto gain = "gain";
        inline constexpr auto bypass = "bypass";

        // Layout helper function
        inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
        {
            juce::AudioProcessorValueTreeState::ParameterLayout layout;
            
            layout.add(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID{ gain, 1 },
                "Gain",
                juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f),
                0.0f
            ));

            layout.add(std::make_unique<juce::AudioParameterBool>(
                juce::ParameterID{ bypass, 1 },
                "Bypass",
                false
            ));

            return layout;
        }
    }
}
