/*
  Real-time FFT Spectrum Analyzer
  Displays frequency spectrum of IQ samples from RTL-SDR
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <array>

class SpectrumAnalyzer : public juce::Component, public juce::Timer
{
public:
    static constexpr int fftOrder = 10;  // 2^10 = 1024 samples
    static constexpr int fftSize = 1 << fftOrder;

    SpectrumAnalyzer();
    ~SpectrumAnalyzer() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // Push IQ samples for FFT analysis
    void pushSamples(const uint8_t* iqData, int numSamples);

    // Set center frequency for display
    void setCenterFrequency(double freqMHz) { centerFrequency = freqMHz; }

    // Set sample rate
    void setSampleRate(double sr) { sampleRate = sr; }

private:
    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize * 2> fftData;  // Complex FFT data
    std::array<float, fftSize> magnitudes;   // FFT magnitudes for display
    std::array<float, fftSize> smoothedMags; // Smoothed for visual stability

    double centerFrequency = 100.0;  // MHz
    double sampleRate = 1024000.0;   // Hz

    int writePosition = 0;
    bool needsUpdate = false;

    const float smoothingFactor = 0.8f;  // IIR smoothing
    const float minDb = -100.0f;
    const float maxDb = 0.0f;

    void performFFT();
    float iqToFloat(uint8_t val) const;  // Convert uint8 IQ to float

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};
