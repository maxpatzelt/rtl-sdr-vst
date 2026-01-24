/*
  Signal Strength Meter (S-Meter)
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>

class SignalMeter : public juce::Component,
                   private juce::Timer
{
public:
    SignalMeter();
    ~SignalMeter() override;

    void updateSignal(const uint8_t* iqData, int numSamples);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    float calculateRMS(const uint8_t* iqData, int numSamples);
    float rmsToDbm(float rms) const;
    float dbmToSUnits(float dbm) const;

    void timerCallback() override;

    float currentDbm = -120.0f;
    float targetDbm = -120.0f;
    float smoothedDbm = -120.0f;

    static constexpr float smoothingFactor = 0.7f;
    static constexpr float minDbm = -130.0f;
    static constexpr float maxDbm = -40.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalMeter)
};
