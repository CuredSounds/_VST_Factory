#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>

#include "DSP/EffectModules/GainProcessor/GainProcessor.h"

namespace
{

constexpr int kNumSamples = 512;
constexpr float kFrequencyHz = 440.0f;
constexpr float kSampleRate = 48000.0f;
constexpr float kAmplitude = 0.25f;
constexpr float kNullTolerance = 1.0e-6f;

std::vector<float> generateSineWave()
{
    std::vector<float> samples (static_cast<std::size_t> (kNumSamples));
    for (int i = 0; i < kNumSamples; ++i)
    {
        const float phase = 2.0f * static_cast<float> (M_PI) * kFrequencyHz
                            * static_cast<float> (i) / kSampleRate;
        samples[static_cast<std::size_t> (i)] = kAmplitude * std::sin (phase);
    }
    return samples;
}

std::vector<float> processThroughGain (const std::vector<float>& input, float gainLinear)
{
    VSTFactory::DSP::GainProcessor processor;
    processor.setGainLinear (gainLinear);

    std::vector<float> output = input;
    float* channels[] = { output.data() };
    processor.processBlock (channels, 1, static_cast<int> (output.size()));
    return output;
}

std::string goldenPath()
{
    return "Tests/golden/gain_null_reference.bin";
}

void writeGolden (const std::vector<float>& data)
{
    std::ofstream out (goldenPath(), std::ios::binary);
    REQUIRE (out.good());
    out.write (reinterpret_cast<const char*> (data.data()),
               static_cast<std::streamsize> (data.size() * sizeof (float)));
}

std::vector<float> readGolden()
{
    std::ifstream in (goldenPath(), std::ios::binary);
    REQUIRE (in.good());

    in.seekg (0, std::ios::end);
    const auto byteSize = in.tellg();
    in.seekg (0, std::ios::beg);

    std::vector<float> data (static_cast<std::size_t> (byteSize) / sizeof (float));
    in.read (reinterpret_cast<char*> (data.data()), byteSize);
    return data;
}

} // namespace

TEST_CASE ("GainProcessor null test against golden reference", "[dsp][null]")
{
    const auto input = generateSineWave();
    const auto output = processThroughGain (input, 1.0f);

    if (std::getenv ("UPDATE_GOLDENS") != nullptr)
    {
        writeGolden (output);
        SUCCEED ("Golden reference updated.");
        return;
    }

    const auto golden = readGolden();
    REQUIRE (golden.size() == output.size());

    for (std::size_t i = 0; i < output.size(); ++i)
        REQUIRE (output[i] == Catch::Approx (golden[i]).margin (kNullTolerance));
}
