#include <catch2/catch_test_macros.hpp>

#include "DSP/EffectModules/GainProcessor/GainProcessor.h"
#include "Helpers/AllocationCounter.h"

TEST_CASE ("GainProcessor processBlock does not allocate heap memory", "[rt][allocation]")
{
    VSTFactory::DSP::GainProcessor processor;
    processor.setGainLinear (2.0f);

    float left[512] {};
    float right[512] {};
    float* channels[] = { left, right };

    for (int i = 0; i < 512; ++i)
    {
        left[i] = 0.1f;
        right[i] = -0.1f;
    }

    VSTFactory::Test::resetAllocationCount();
    VSTFactory::Test::setCountingEnabled (true);

    for (int block = 0; block < 8; ++block)
        processor.processBlock (channels, 2, 512);

    VSTFactory::Test::setCountingEnabled (false);

    REQUIRE (VSTFactory::Test::getAllocationCount() == 0);
}
