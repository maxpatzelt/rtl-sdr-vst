/*
  Audio Resampler Implementation
*/

#include "AudioResampler.h"

AudioResampler::AudioResampler()
{
    reset();
}

AudioResampler::~AudioResampler()
{
}

void AudioResampler::setRates(double inputRate, double outputRate)
{
    inputSampleRate = inputRate;
    outputSampleRate = outputRate;
    ratio = outputRate / inputRate;
}

void AudioResampler::reset()
{
    phase = 0.0;
    lastSample = 0.0f;
    interpolator.reset();
}

int AudioResampler::getOutputSize(int inputSize) const
{
    return (int)(inputSize * ratio) + 2; // +2 for safety margin
}

void AudioResampler::process(const std::vector<float>& input, std::vector<float>& output)
{
    if (input.empty())
        return;

    // Calculate how many output samples we'll produce
    int outputSamples = getOutputSize((int)input.size());

    // Prepare output buffer
    std::vector<float> tempOutput(outputSamples);

    // Use JUCE's Lagrange interpolator for high-quality resampling
    // This is similar to approach in SDR++ for audio output
    int numOutputSamples = interpolator.process(
        ratio,
        input.data(),
        tempOutput.data(),
        outputSamples,
        (int)input.size(),
        0  // Max input samples to use (0 = all)
    );

    // Append to output
    output.insert(output.end(), tempOutput.begin(), tempOutput.begin() + numOutputSamples);
}
