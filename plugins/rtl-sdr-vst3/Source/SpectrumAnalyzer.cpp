/*
  Spectrum Analyzer — Phosphor Amber Style
  Renders the FFT magnitude as a warm amber trace on a near-black phosphor-tube
  background, matching the retro ham radio aesthetic of the plugin.
*/

#include "SpectrumAnalyzer.h"
#include <cmath>

// ── Palette ───────────────────────────────────────────────────────────────────
namespace {
    const juce::Colour BG_DARK   { 0xFF030604 };  // near-black display background
    const juce::Colour GRID_DIM  { 0xFF0C1208 };  // very dim grid lines
    const juce::Colour GRID_MID  { 0xFF142010 };  // slightly brighter grid
    const juce::Colour AMBER_HOT { 0xFFFFB300 };  // amber — peak trace
    const juce::Colour AMBER_DIM { 0xFFA06000 };  // dimmer amber — fill base
    const juce::Colour PHOSPHOR  { 0xFF39FF14 };  // phosphor green — accent
    const juce::Colour LABEL_COL { 0xFF7A7050 };  // dim cream — frequency labels
    const juce::Colour MARKER    { 0xFFFF4020 };  // red centre marker
}

//==============================================================================
SpectrumAnalyzer::SpectrumAnalyzer()
    : fft    (fftOrder),
      window (fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    fftData.fill (0.0f);
    magnitudes.fill (minDb);
    smoothedMags.fill (minDb);

    startTimerHz (30);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
    stopTimer();
}

void SpectrumAnalyzer::pushSamples (const uint8_t* iqData, int numSamples)
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
        needsUpdate   = true;
    }
}

float SpectrumAnalyzer::iqToFloat (uint8_t val) const
{
    return (static_cast<float>(val) - 127.5f) / 127.5f;
}

void SpectrumAnalyzer::performFFT()
{
    window.multiplyWithWindowingTable (fftData.data(), fftSize);
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    for (int i = 0; i < fftSize / 2; ++i)
    {
        float mag  = std::sqrt (fftData[i * 2] * fftData[i * 2]
                               + fftData[i * 2 + 1] * fftData[i * 2 + 1]);
        float db   = mag > 0.0f ? 20.0f * std::log10 (mag) : minDb;
        magnitudes[i] = juce::jlimit (minDb, maxDb, db);
    }
}

void SpectrumAnalyzer::timerCallback()
{
    if (needsUpdate)
    {
        for (int i = 0; i < fftSize / 2; ++i)
            smoothedMags[i] = smoothingFactor * smoothedMags[i]
                            + (1.0f - smoothingFactor) * magnitudes[i];
        needsUpdate = false;
        repaint();
    }
}

//==============================================================================
void SpectrumAnalyzer::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    const float W  = (float)bounds.getWidth();
    const float H  = (float)bounds.getHeight();
    const int   N  = fftSize / 2;

    // ── Background — phosphor tube ──────────────────────────────────────────
    g.fillAll (BG_DARK);

    // Inner vignette (darker edges)
    {
        juce::ColourGradient vgn (juce::Colours::transparentBlack, W * 0.5f, H * 0.5f,
                                   juce::Colour (0xFF010302).withAlpha (0.7f), 0.0f, 0.0f, true);
        g.setGradientFill (vgn);
        g.fillRect (bounds);
    }

    // ── Grid ────────────────────────────────────────────────────────────────
    // Horizontal dB lines
    for (int db = -10; db >= -90; db -= 10)
    {
        const float norm = juce::jmap ((float)db, minDb, maxDb, 1.0f, 0.0f);
        const float y    = H * norm;
        const bool  major = (db % 20 == 0);
        g.setColour (major ? GRID_MID : GRID_DIM);
        g.drawHorizontalLine ((int)y, 0.0f, W);
    }

    // Vertical frequency lines
    for (int i = 1; i < 8; ++i)
    {
        const float x = W * i / 8.0f;
        g.setColour (GRID_DIM);
        g.drawVerticalLine ((int)x, 0.0f, H);
    }

    // ── Spectrum path ────────────────────────────────────────────────────────
    juce::Path tracePath;
    bool started = false;

    for (int i = 0; i < N; ++i)
    {
        const float x    = juce::jmap ((float)i, 0.0f, (float)N, 0.0f, W);
        const float norm = juce::jmap (smoothedMags[i], minDb, maxDb, 1.0f, 0.0f);
        const float y    = H * norm;

        if (!started) { tracePath.startNewSubPath (x, y); started = true; }
        else            tracePath.lineTo (x, y);
    }

    // Fill under curve with amber gradient
    juce::Path fill = tracePath;
    fill.lineTo (W, H);
    fill.lineTo (0.0f, H);
    fill.closeSubPath();

    juce::ColourGradient fillGrad (
        AMBER_DIM.withAlpha (0.22f), 0.0f, 0.0f,
        AMBER_DIM.withAlpha (0.0f),  0.0f, H, false);
    g.setGradientFill (fillGrad);
    g.fillPath (fill);

    // Main amber trace
    g.setColour (AMBER_HOT.withAlpha (0.9f));
    g.strokePath (tracePath, juce::PathStrokeType (1.6f));

    // Second pass: phosphor green highlight on hottest parts
    juce::Path hotPath;
    bool hpStarted = false;
    for (int i = 0; i < N; ++i)
    {
        if (smoothedMags[i] > -20.0f)
        {
            const float x    = juce::jmap ((float)i, 0.0f, (float)N, 0.0f, W);
            const float norm = juce::jmap (smoothedMags[i], minDb, maxDb, 1.0f, 0.0f);
            const float y    = H * norm;
            if (!hpStarted) { hotPath.startNewSubPath (x, y); hpStarted = true; }
            else              hotPath.lineTo (x, y);
        }
        else if (hpStarted)
        {
            g.setColour (PHOSPHOR.withAlpha (0.75f));
            g.strokePath (hotPath, juce::PathStrokeType (1.2f));
            hotPath.clear();
            hpStarted = false;
        }
    }
    if (hpStarted)
    {
        g.setColour (PHOSPHOR.withAlpha (0.75f));
        g.strokePath (hotPath, juce::PathStrokeType (1.2f));
    }

    // ── Centre frequency marker ──────────────────────────────────────────────
    g.setColour (MARKER.withAlpha (0.45f));
    g.drawVerticalLine ((int)(W * 0.5f), 0.0f, H);

    // ── Frequency labels ─────────────────────────────────────────────────────
    g.setColour (LABEL_COL);
    g.setFont   (juce::Font (juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::plain));

    const double span     = sampleRate / 1e6;       // MHz visible
    const double startMHz = centerFrequency - span / 2.0;
    const double endMHz   = centerFrequency + span  / 2.0;

    g.drawText (juce::String (startMHz, 2) + " MHz", 4,         (int)H - 14, 70, 12,
                juce::Justification::left);
    g.drawText (juce::String (centerFrequency, 3) + " MHz",
                (int)(W * 0.5f) - 30, (int)H - 14, 60, 12, juce::Justification::centred);
    g.drawText (juce::String (endMHz, 2) + " MHz",
                (int)W - 72,          (int)H - 14, 70, 12, juce::Justification::right);

    // dB scale (right side)
    g.setColour (LABEL_COL.withAlpha (0.6f));
    for (int db = -20; db >= -80; db -= 20)
    {
        const float norm = juce::jmap ((float)db, minDb, maxDb, 1.0f, 0.0f);
        const float y    = H * norm;
        g.drawText (juce::String (db) + "dB", (int)W - 34, (int)y - 7, 32, 12,
                    juce::Justification::right);
    }

    // ── Outer frame ──────────────────────────────────────────────────────────
    g.setColour (juce::Colour (0xFF1A1812));
    g.drawRect (bounds, 1);
}

void SpectrumAnalyzer::resized() {}

void SpectrumAnalyzer::mouseDown (const juce::MouseEvent& event)
{
    if (!onFrequencyClicked) return;

    const float relX = (float)event.getPosition().getX() / (float)getWidth();
    const double span = sampleRate / 1e6;
    double freq = centerFrequency + (relX - 0.5) * span;
    freq = juce::jlimit (24.0, 1766.0, freq);
    onFrequencyClicked (freq);
}
