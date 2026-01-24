/*
  Signal Strength Meter Implementation
*/

#include "SignalMeter.h"
#include <cmath>

SignalMeter::SignalMeter()
{
    startTimerHz(30);  // 30 FPS refresh
}

SignalMeter::~SignalMeter()
{
    stopTimer();
}

void SignalMeter::updateSignal(const uint8_t* iqData, int numSamples)
{
    if (!iqData || numSamples == 0) return;

    float rms = calculateRMS(iqData, numSamples);
    targetDbm = rmsToDbm(rms);
}

float SignalMeter::calculateRMS(const uint8_t* iqData, int numSamples)
{
    float sumSquares = 0.0f;
    int count = 0;

    for (int i = 0; i < numSamples; i += 2)
    {
        float I = (static_cast<float>(iqData[i]) - 127.5f) / 127.5f;
        float Q = (static_cast<float>(iqData[i + 1]) - 127.5f) / 127.5f;

        sumSquares += I * I + Q * Q;
        count++;
    }

    return count > 0 ? std::sqrt(sumSquares / count) : 0.0f;
}

float SignalMeter::rmsToDbm(float rms) const
{
    if (rms < 1e-6f) return minDbm;

    // Convert RMS to dBm (approximate)
    float db = 20.0f * std::log10(rms);
    float dbm = db - 20.0f;  // Calibration offset

    return juce::jlimit(minDbm, maxDbm, dbm);
}

float SignalMeter::dbmToSUnits(float dbm) const
{
    // S-meter scale:
    // S0 = -127 dBm
    // S1 = -121 dBm
    // S2 = -115 dBm
    // ...
    // S9 = -73 dBm
    // S9+10 = -63 dBm
    // S9+20 = -53 dBm
    // S9+30 = -43 dBm

    if (dbm <= -127.0f) return 0.0f;
    if (dbm >= -43.0f) return 12.0f;  // S9+30

    if (dbm < -73.0f)
    {
        // S0 to S9
        return juce::jmap(dbm, -127.0f, -73.0f, 0.0f, 9.0f);
    }
    else
    {
        // S9+ (10dB increments)
        return 9.0f + juce::jmap(dbm, -73.0f, -43.0f, 0.0f, 3.0f);
    }
}

void SignalMeter::timerCallback()
{
    // Smooth the reading
    smoothedDbm = smoothedDbm * smoothingFactor + targetDbm * (1.0f - smoothingFactor);
    repaint();
}

void SignalMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(2);

    // Background
    g.fillAll(juce::Colour(0xFF1a1a1a));

    // Border
    g.setColour(juce::Colour(0xFF3a3a3a));
    g.drawRect(bounds);

    // S-units scale
    float sUnits = dbmToSUnits(smoothedDbm);

    // Draw meter bar
    auto meterBounds = bounds.reduced(4);
    int meterWidth = (int)(meterBounds.getWidth() * (sUnits / 12.0f));

    // Color gradient based on signal strength
    juce::Colour meterColor;
    if (sUnits < 3.0f)
        meterColor = juce::Colours::red;
    else if (sUnits < 5.0f)
        meterColor = juce::Colours::orange;
    else if (sUnits < 7.0f)
        meterColor = juce::Colours::yellow;
    else
        meterColor = juce::Colours::green;

    g.setColour(meterColor);
    g.fillRect(meterBounds.getX(), meterBounds.getY(), meterWidth, meterBounds.getHeight());

    // Draw scale markers
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    for (int i = 1; i <= 9; ++i)
    {
        int x = (int)(meterBounds.getX() + meterBounds.getWidth() * (i / 12.0f));
        g.drawVerticalLine(x, meterBounds.getY(), meterBounds.getBottom());
    }

    // S9+ markers
    for (int i = 10; i <= 12; ++i)
    {
        int x = (int)(meterBounds.getX() + meterBounds.getWidth() * (i / 12.0f));
        g.setColour(juce::Colours::red.withAlpha(0.3f));
        g.drawVerticalLine(x, meterBounds.getY(), meterBounds.getBottom());
    }

    // Text labels
    g.setColour(juce::Colours::white);
    g.setFont(10.0f);

    // S-units
    for (int i = 1; i <= 9; ++i)
    {
        int x = (int)(meterBounds.getX() + meterBounds.getWidth() * (i / 12.0f));
        g.drawText(juce::String(i), x - 5, meterBounds.getBottom() + 2, 10, 10, juce::Justification::centred);
    }

    // S9+
    g.drawText("+20", (int)(meterBounds.getX() + meterBounds.getWidth() * (10.0f / 12.0f)) - 8,
               meterBounds.getBottom() + 2, 20, 10, juce::Justification::centred);

    // Current reading
    juce::String reading;
    if (sUnits < 9.0f)
        reading = "S" + juce::String((int)sUnits);
    else
        reading = "S9+" + juce::String((int)((sUnits - 9.0f) * 10.0f));

    reading += " (" + juce::String(smoothedDbm, 1) + " dBm)";

    g.setFont(11.0f);
    g.drawText(reading, bounds.reduced(4), juce::Justification::centredTop);
}

void SignalMeter::resized()
{
    // Nothing to resize
}
