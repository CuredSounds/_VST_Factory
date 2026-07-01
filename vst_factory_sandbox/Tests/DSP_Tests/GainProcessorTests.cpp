#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

#include "DSP/EffectModules/GainProcessor/GainProcessor.h"

TEST_CASE ("GainProcessor applies linear gain", "[dsp][gain]")
{
    VSTFactory::DSP::GainProcessor processor;
    processor.setGainLinear (2.0f);

    float left[] = { 0.5f, -0.25f };
    float right[] = { 0.25f, 0.25f };
    float* channels[] = { left, right };

    processor.processBlock (channels, 2, 2);

    REQUIRE (left[0] == Catch::Approx (1.0f));
    REQUIRE (left[1] == Catch::Approx (-0.5f));
    REQUIRE (right[0] == Catch::Approx (0.5f));
    REQUIRE (right[1] == Catch::Approx (0.5f));
}

TEST_CASE ("GainProcessor bypass skips processing", "[dsp][gain]")
{
    VSTFactory::DSP::GainProcessor processor;
    processor.setGainLinear (4.0f);
    processor.setBypassed (true);

    float sample = 0.25f;
    float* channels[] = { &sample };

    processor.processBlock (channels, 1, 1);

    REQUIRE (sample == Catch::Approx (0.25f));
}

TEST_CASE ("GainProcessor unity gain is a no-op", "[dsp][gain]")
{
    VSTFactory::DSP::GainProcessor processor;
    processor.setGainLinear (1.0f);

    float sample = -0.125f;
    float* channels[] = { &sample };

    processor.processBlock (channels, 1, 1);

    REQUIRE (sample == Catch::Approx (-0.125f));
}
