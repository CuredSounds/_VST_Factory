#include "GainProcessor.h"

namespace VSTFactory::DSP
{

void GainProcessor::processBlock (float* const* channelData, int numChannels, int numSamples) noexcept
{
    if (bypassed_ || gainLinear_ == 1.0f)
        return;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = channelData[ch];
        for (int i = 0; i < numSamples; ++i)
            data[i] *= gainLinear_;
    }
}

} // namespace VSTFactory::DSP
