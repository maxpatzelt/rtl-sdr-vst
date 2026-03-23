/*
  Signal Meter — Vintage S-Meter Style
  Drawn as a classic ham-radio S-meter face:
    - Cream/ivory face with engraved scale
    - S0–S9 scale in black, dB-over-S9 in red zone
    - Amber illuminated needle with trail shadow
    - Bakelite-style housing
*/

#include "SignalMeter.h"
#include <cmath>

SignalMeter::SignalMeter()
{
    startTimerHz (30);
}

SignalMeter::~SignalMeter()
{
    stopTimer();
}

void SignalMeter::updateSignal (const uint8_t* iqData, int numSamples)
{
    if (!iqData || numSamples == 0) return;
    targetDbm = rmsToDbm (calculateRMS (iqData, numSamples));
}

float SignalMeter::calculateRMS (const uint8_t* iqData, int numSamples)
{
    float sum = 0.0f;
    int   cnt = 0;
    for (int i = 0; i < numSamples; i += 2)
    {
        const float I = (static_cast<float>(iqData[i])     - 127.5f) / 127.5f;
        const float Q = (static_cast<float>(iqData[i + 1]) - 127.5f) / 127.5f;
        sum += I * I + Q * Q;
        ++cnt;
    }
    return cnt > 0 ? std::sqrt (sum / cnt) : 0.0f;
}

float SignalMeter::rmsToDbm (float rms) const
{
    if (rms < 1e-6f) return minDbm;
    return juce::jlimit (minDbm, maxDbm, 20.0f * std::log10 (rms) - 20.0f);
}

float SignalMeter::dbmToNorm (float dbm) const
{
    // Map: -127 dBm (S0) → 0.0 … -73 dBm (S9) → 0.75 … -43 dBm (S9+30) → 1.0
    if (dbm <= -127.0f) return 0.0f;
    if (dbm <= -73.0f)  return juce::jmap (dbm, -127.0f, -73.0f, 0.0f, 0.75f);
    if (dbm <= -43.0f)  return juce::jmap (dbm, -73.0f,  -43.0f, 0.75f, 1.0f);
    return 1.0f;
}

float SignalMeter::dbmToSUnits (float dbm) const
{
    if (dbm <= -127.0f) return 0.0f;
    if (dbm < -73.0f)   return juce::jmap (dbm, -127.0f, -73.0f, 0.0f, 9.0f);
    return 9.0f + juce::jmap (dbm, -73.0f, -43.0f, 0.0f, 3.0f);
}

void SignalMeter::timerCallback()
{
    smoothedDbm = smoothedDbm * smoothingFactor + targetDbm * (1.0f - smoothingFactor);
    repaint();
}

//==============================================================================
void SignalMeter::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    const float W = (float)bounds.getWidth();
    const float H = (float)bounds.getHeight();

    // ── Meter housing (dark bakelite frame) ───────────────────────────────────
    g.setColour (juce::Colour (0xFF1A1812));
    g.fillRoundedRectangle (bounds.toFloat(), 3.0f);
    g.setColour (juce::Colour (0xFF2E2C20));
    g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f), 3.0f, 1.0f);

    // ── Meter face (ivory/cream) ───────────────────────────────────────────────
    const auto face = bounds.reduced (3);
    {
        juce::ColourGradient faceGrad (
            juce::Colour (0xFFE8E0C0), (float)face.getX(), (float)face.getY(),
            juce::Colour (0xFFCDC5A5), (float)face.getX(), (float)face.getBottom(),
            false);
        g.setGradientFill (faceGrad);
        g.fillRoundedRectangle (face.toFloat(), 2.0f);
    }

    // Red zone at the right (S9+)
    const float s9x = face.getX() + face.getWidth() * 0.75f;
    {
        juce::ColourGradient redZone (
            juce::Colour (0x00FF2010), s9x, 0.0f,
            juce::Colour (0x33FF2010), (float)face.getRight(), 0.0f,
            false);
        g.setGradientFill (redZone);
        g.fillRect (s9x, (float)face.getY(), (float)face.getRight() - s9x, (float)face.getHeight());
    }

    // ── Scale marks & labels ──────────────────────────────────────────────────
    // The scale runs from 0 to 1.0 (normalised) = left edge to right edge of face
    const float labelY  = (float)face.getBottom() - 14.0f;
    const float tickTop = (float)face.getY() + 5.0f;

    // S-unit ticks (S1–S9)
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 8.0f, juce::Font::plain));
    for (int s = 1; s <= 9; ++s)
    {
        const float norm = (float)s / 12.0f;  // S1-S9 occupy 0-0.75 of scale
        const float xPos = face.getX() + face.getWidth() * (norm * 0.75f / 0.75f);

        // Tick mark
        const bool major = (s % 3 == 0);
        g.setColour (juce::Colour (0xFF2A2410));
        g.fillRect (xPos - 0.5f, tickTop, 1.0f, major ? 8.0f : 5.0f);

        // Label for major ticks
        if (major)
        {
            g.setColour (juce::Colour (0xFF1A1408));
            g.drawText (juce::String (s), (int)xPos - 6, (int)labelY, 12, 11,
                        juce::Justification::centred);
        }
    }

    // S9+ marks (+10, +20, +30)
    const int plusVals[] = { 10, 20, 30 };
    for (int i = 0; i < 3; ++i)
    {
        const float norm = 0.75f + (i + 1) * (0.25f / 3.0f);
        const float xPos = face.getX() + face.getWidth() * norm;
        g.setColour (juce::Colour (0xFF8A1010));
        g.fillRect (xPos - 0.5f, tickTop, 1.0f, 8.0f);
        g.drawText ("+" + juce::String (plusVals[i]),
                    (int)xPos - 10, (int)labelY, 20, 11, juce::Justification::centred);
    }

    // "S" label at far left
    g.setColour (juce::Colour (0xFF1A1408));
    g.setFont   (juce::Font (juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::bold));
    g.drawText  ("S", face.getX() + 2, (int)labelY, 10, 11, juce::Justification::left);

    // ── Needle ────────────────────────────────────────────────────────────────
    const float normPos  = dbmToNorm (smoothedDbm);
    const float needleX  = face.getX() + face.getWidth() * normPos;
    const float pivotY   = (float)face.getBottom() + 8.0f;  // pivot below face
    const float needleTip = (float)face.getY() + 6.0f;

    // Needle shadow
    g.setColour (juce::Colour (0x30000000));
    g.drawLine  (needleX + 1.5f, pivotY, needleX + 1.5f, needleTip, 1.5f);

    // Needle body — amber
    g.setColour (juce::Colour (0xFFD07800));
    g.drawLine  (needleX, pivotY, needleX, needleTip + 4.0f, 2.0f);

    // Needle tip — brighter amber
    g.setColour (juce::Colour (0xFFFFB300));
    g.drawLine  (needleX, needleTip + 4.0f, needleX, needleTip, 1.5f);

    // Pivot screw
    g.setColour (juce::Colour (0xFF5A5040));
    g.fillEllipse (needleX - 3.5f, pivotY - 3.5f, 7.0f, 7.0f);

    // ── Reading text ──────────────────────────────────────────────────────────
    const float sUnits = dbmToSUnits (smoothedDbm);
    juce::String reading;
    if (sUnits < 9.1f)
        reading = "S" + juce::String ((int)sUnits) + "  (" + juce::String (smoothedDbm, 0) + " dBm)";
    else
        reading = "S9+" + juce::String ((int)((sUnits - 9.0f) * 10.0f))
                + "  (" + juce::String (smoothedDbm, 0) + " dBm)";

    g.setColour (juce::Colour (0xFF3A3020));
    g.setFont   (juce::Font (juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
    g.drawText  (reading, face.getX() + 2, face.getY() + 2,
                 face.getWidth() - 4, 12, juce::Justification::centred);
}

void SignalMeter::resized() {}
