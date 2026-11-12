/*
  Waterfall Display — Phosphor Green Theme
  Time-scrolling spectrum history coloured as a phosphor CRT tube:
    weak signal → deep charcoal-green
    mid signal  → bright phosphor green
    hot signal  → amber/gold peak
  Complements the amber spectrum analyzer above it.
*/

#include "WaterfallDisplay.h"
#include <cmath>

WaterfallDisplay::WaterfallDisplay()
    : fft    (fftOrder),
      window (fftSize, juce::dsp::WindowingFunction<float>::hann),
      history (historyLines)
{
    fftData.fill (0.0f);
    for (auto& line : history)
        line.fill (-100.0f);

    waterfallImage = juce::Image (juce::Image::RGB, fftSize / 2, historyLines, true);

    startTimerHz (25);
}

WaterfallDisplay::~WaterfallDisplay()
{
    stopTimer();
}

void WaterfallDisplay::pushSamples (const uint8_t* iqData, int numSamples)
{
    if (!iqData || numSamples == 0) return;

    for (int i = 0; i < numSamples && writePosition < fftSize; i += 2)
    {
        fftData[writePosition * 2]     = iqToFloat (iqData[i]);
        fftData[writePosition * 2 + 1] = iqToFloat (iqData[i + 1]);
        ++writePosition;
    }

    if (writePosition >= fftSize)
    {
        performFFT();
        writePosition = 0;
    }
}

float WaterfallDisplay::iqToFloat (uint8_t val) const
{
    return (static_cast<float>(val) - 127.5f) / 127.5f;
}

void WaterfallDisplay::performFFT()
{
    window.multiplyWithWindowingTable (fftData.data(), fftSize);
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    currentLine = (currentLine + 1) % historyLines;

    for (int i = 0; i < fftSize / 2; ++i)
    {
        const float mag = std::sqrt (fftData[i * 2] * fftData[i * 2]
                                    + fftData[i * 2 + 1] * fftData[i * 2 + 1]);
        const float db  = mag > 0.0f ? 20.0f * std::log10 (mag) : -100.0f;
        history[currentLine][i] = juce::jlimit (-100.0f, 0.0f, db);
    }

    updateWaterfall();
}

// ── Phosphor green colormap ───────────────────────────────────────────────────
// 0.0 (silent) → near-black dark green
// 0.4 (mid)    → bright phosphor green
// 0.7 (loud)   → yellow-green
// 1.0 (peak)   → amber/gold
static juce::Colour phosphorColor (float normalized)
{
    normalized = juce::jlimit (0.0f, 1.0f, normalized);

    if (normalized < 0.15f)
    {
        // Black → very dark phosphor green
        const float t = normalized / 0.15f;
        return juce::Colour::fromRGB (
            (uint8_t)(t * 4),
            (uint8_t)(t * 22),
            (uint8_t)(t * 4));
    }
    else if (normalized < 0.5f)
    {
        // Dark green → bright phosphor green
        const float t = (normalized - 0.15f) / 0.35f;
        return juce::Colour::fromRGB (
            (uint8_t)(4  + t * 53),
            (uint8_t)(22 + t * 233),
            (uint8_t)(4  + t * 16));
    }
    else if (normalized < 0.78f)
    {
        // Bright green → yellow-green
        const float t = (normalized - 0.5f) / 0.28f;
        return juce::Colour::fromRGB (
            (uint8_t)(57  + t * 198),
            (uint8_t)(255),
            (uint8_t)(20  - t * 20));
    }
    else
    {
        // Yellow-green → amber
        const float t = (normalized - 0.78f) / 0.22f;
        return juce::Colour::fromRGB (
            (uint8_t)(255),
            (uint8_t)(255 - t * 79),   // 255 → 176 (amber)
            0);
    }
}

void WaterfallDisplay::updateWaterfall()
{
    for (int y = 0; y < historyLines; ++y)
    {
        const int lineIdx = (currentLine - y + historyLines) % historyLines;

        for (int x = 0; x < fftSize / 2; ++x)
        {
            const float db         = history[lineIdx][x];
            const float normalized = juce::jmap (db, -100.0f, 0.0f, 0.0f, 1.0f);
            waterfallImage.setPixelAt (x, y, phosphorColor (normalized));
        }
    }
}

void WaterfallDisplay::timerCallback()
{
    repaint();
}

void WaterfallDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    const float W = (float)bounds.getWidth();
    const float H = (float)bounds.getHeight();

    // Waterfall image scaled to component bounds
    g.drawImage (waterfallImage, bounds.toFloat(),
                 juce::RectanglePlacement::stretchToFit);

    // ── Centre marker ──────────────────────────────────────────────────────────
    g.setColour (juce::Colour (0xFFFF2020).withAlpha (0.35f));
    g.drawVerticalLine ((int)(W * 0.5f), 0.0f, H);

    // ── Sparse vertical grid lines ─────────────────────────────────────────────
    g.setColour (juce::Colour (0xFF000000).withAlpha (0.25f));
    for (int i = 1; i < 8; ++i)
        g.drawVerticalLine ((int)(W * i / 8.0f), 0.0f, H);

    // ── Frequency labels ──────────────────────────────────────────────────────
    g.setColour (juce::Colour (0xFFFFB300).withAlpha (0.55f));
    g.setFont   (juce::Font (juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain));

    const double span     = sampleRate / 1e6;
    const double startMHz = centerFrequency - span / 2.0;
    const double endMHz   = centerFrequency + span / 2.0;

    g.drawText (juce::String (startMHz, 2) + " MHz",
                3,              2, 64, 11, juce::Justification::left);
    g.drawText (juce::String (endMHz, 2) + " MHz",
                (int)W - 66,   2, 64, 11, juce::Justification::right);

    // ── Outer frame ───────────────────────────────────────────────────────────
    g.setColour (juce::Colour (0xFF1A1812));
    g.drawRect  (bounds, 1);
}

void WaterfallDisplay::resized() {}
