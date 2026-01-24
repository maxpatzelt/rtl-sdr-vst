/*
  SSB Demodulator - Single Sideband (USB/LSB)
  For amateur radio and shortwave communications
*/

#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>

class SSBDemodulator
{
public:
    enum Mode
    {
        USB,  // Upper Sideband
        LSB   // Lower Sideband
    };

    SSBDemodulator();
    ~SSBDemodulator();

    void setSampleRate(double sampleRate);
    void setMode(Mode mode) { ssbMode = mode; }

    // Process IQ samples and output demodulated audio
    void processBlock(const uint8_t* iqData, int numIQBytes, std::vector<float>& output);

    // Set fine tuning offset (+/- 10 kHz)
    void setFineTuneOffset(double offsetHz) { fineTuneOffset = offsetHz; }

    // AGC settings
    void setAGCEnabled(bool enabled) { agcEnabled = enabled; }
    void setAGCSpeed(float speed) { agcSpeed = speed; }

private:
    double sampleRate = 1024000.0;
    Mode ssbMode = USB;
    double fineTuneOffset = 0.0;  // Hz

    // AGC
    bool agcEnabled = true;
    float agcGain = 1.0f;
    float agcSpeed = 0.01f;
    float agcTarget = 0.3f;

    // Hilbert transform for SSB
    juce::dsp::FIR::Filter<float> hilbertFilter;
    std::vector<float> hilbertCoefficients;

    // Band-pass filter
    juce::dsp::IIR::Filter<float> bpFilter;

    // DC blocking
    float dcOffset = 0.0f;
    float dcAlpha = 0.999f;

    // IQ to float conversion
    float iqToFloat(uint8_t val) const;

    // SSB detection using Hilbert transform
    float processSSB(float I, float Q);

    // DC removal
    float removeDC(float sample);

    // AGC
    float applyAGC(float sample);

    // Initialize Hilbert filter coefficients
    void initializeHilbertFilter();

    // Initialize band-pass filter
    void initializeBandPassFilter();
};
