/*
  AM Demodulator - Envelope detection for amplitude modulation
*/

#pragma once

#include <cmath>
#include <vector>

class AMDemodulator
{
public:
    AMDemodulator();
    ~AMDemodulator();

    void setSampleRate(double sampleRate);

    // Process IQ samples and output demodulated audio
    float process(uint8_t I, uint8_t Q);

    // Process a block of IQ samples
    void processBlock(const uint8_t* iqData, int numIQBytes, std::vector<float>& output);

    void setAGCEnabled(bool enabled) { agcEnabled = enabled; }
    void setAGCSpeed(float speed) { agcSpeed = speed; }
    void setDCBlockEnabled(bool enabled) { dcBlockEnabled = enabled; }

private:
    double sampleRate = 1024000.0;

    // AGC (Automatic Gain Control)
    bool agcEnabled = true;
    float agcGain = 1.0f;
    float agcSpeed = 0.01f;
    float agcTarget = 0.5f;

    // DC blocking filter
    bool dcBlockEnabled = true;
    float dcOffset = 0.0f;
    float dcAlpha = 0.999f;

    // IQ to float conversion
    float iqToFloat(uint8_t val) const;

    // Envelope detection
    float calculateEnvelope(float I, float Q);

    // DC blocking
    float removeDC(float sample);

    // AGC
    float applyAGC(float sample);
};
