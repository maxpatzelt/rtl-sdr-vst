/*
  Waterfall Display - Time-domain spectrum visualization
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <vector>

class WaterfallDisplay : public juce::Component,
                        private juce::Timer
{
public:
    WaterfallDisplay();
    ~WaterfallDisplay() override;

    void pushSamples(const uint8_t* iqData, int numSamples);

    void setCenterFrequency(double freqHz) { centerFrequency = freqHz / 1e6; }
    void setSampleRate(double rate) { sampleRate = rate; }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;  // 1024
    static constexpr int historyLines = 200;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize * 2> fftData;
    std::vector<std::array<float, fftSize / 2>> history;

    int writePosition = 0;
    int currentLine = 0;

    double centerFrequency = 100.0;  // MHz
    double sampleRate = 1024000.0;   // Hz

    juce::Image waterfallImage;

    float iqToFloat(uint8_t val) const;
    void performFFT();
    void updateWaterfall();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaterfallDisplay)
};
