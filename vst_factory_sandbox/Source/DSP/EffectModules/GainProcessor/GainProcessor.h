#pragma once

#include <cstddef>

namespace VSTFactory::DSP
{

/** Pure gain effect — no JUCE dependencies, unit-testable in isolation. */
class GainProcessor
{
public:
    void setGainLinear (float gainLinear) noexcept { gainLinear_ = gainLinear; }
    void setBypassed (bool bypassed) noexcept { bypassed_ = bypassed; }

    float getGainLinear() const noexcept { return gainLinear_; }
    bool isBypassed() const noexcept { return bypassed_; }

    void processBlock (float* const* channelData, int numChannels, int numSamples) noexcept;

private:
    float gainLinear_ = 1.0f;
    bool bypassed_ = false;
};

} // namespace VSTFactory::DSP
