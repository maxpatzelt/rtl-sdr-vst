/*
  Audio Resampler
  Converts SDR sample rate to audio sample rate

  Inspired by:
  - rtl_fm: Decimation and resampling approach
  - SDRPlusPlus: Polyphase resampler
  - JUCE Resampler: High-quality interpolation
*/

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

class AudioResampler
{
public:
    AudioResampler();
    ~AudioResampler();

    // Set input (SDR) and output (audio) sample rates
    void setRates(double inputRate, double outputRate);

    // Process samples (simple linear interpolation for now)
    // Input: samples at SDR rate
    // Output: samples at audio rate (appended to output vector)
    void process(const std::vector<float>& input, std::vector<float>& output);

    // Reset state
    void reset();

    // Get expected output size for given input size
    int getOutputSize(int inputSize) const;

private:
    double inputSampleRate = 1024000.0;
    double outputSampleRate = 48000.0;
    double ratio = 1.0;

    // Resampling state
    double phase = 0.0;
    float lastSample = 0.0f;

    // JUCE's built-in resampler for high quality
    juce::LagrangeInterpolator interpolator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioResampler)
};
