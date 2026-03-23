/*
  RTL-SDR Plugin Editor v3.0 — Retro Ham Radio Aesthetic
  Kenwood/Heathkit/Collins inspired front-panel design.

  Layout (860 × 620 px):
    [0]   Header bar — rivets, title, status LED            48 px
    [48]  VFO 7-segment + S-meter                           85 px
    [133] Spectrum analyzer (phosphor amber)               115 px
    [248] Waterfall (phosphor green)                        90 px
    [338] Mode buttons + Band presets                       48 px
    [386] Knob row (GAIN / VOL / SQL / FINE / SAT + AGC)  110 px
    [496] Device selector + transport                       36 px
    [532] Preset selector + history                         36 px
    [568] Footer                                            32 px
    Total: 600 px
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
// RetroLookAndFeel palette

const juce::Colour RetroLookAndFeel::CHASSIS_BG  { 0xFF0D0D09 };
const juce::Colour RetroLookAndFeel::PANEL_FACE  { 0xFF18180F };
const juce::Colour RetroLookAndFeel::DISPLAY_BG  { 0xFF030604 };
const juce::Colour RetroLookAndFeel::AMBER       { 0xFFFFB300 };
const juce::Colour RetroLookAndFeel::AMBER_DIM   { 0xFF1A0D00 };
const juce::Colour RetroLookAndFeel::PHOSPHOR    { 0xFF39FF14 };
const juce::Colour RetroLookAndFeel::CREAM       { 0xFFCFC9A8 };
const juce::Colour RetroLookAndFeel::KNOB_DARK   { 0xFF1E1B12 };
const juce::Colour RetroLookAndFeel::STEEL       { 0xFF3A3828 };

//==============================================================================
RetroLookAndFeel::RetroLookAndFeel()
{
    // Override default colours
    setColour (juce::ResizableWindow::backgroundColourId,   CHASSIS_BG);
    setColour (juce::ComboBox::backgroundColourId,          PANEL_FACE);
    setColour (juce::ComboBox::outlineColourId,             STEEL);
    setColour (juce::ComboBox::textColourId,                CREAM);
    setColour (juce::ComboBox::arrowColourId,               AMBER);
    setColour (juce::PopupMenu::backgroundColourId,         juce::Colour (0xFF151510));
    setColour (juce::PopupMenu::textColourId,               CREAM);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, STEEL);
    setColour (juce::PopupMenu::highlightedTextColourId,    AMBER);
}

void RetroLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                         int x, int y, int width, int height,
                                         float sliderPos,
                                         float startAngle, float endAngle,
                                         juce::Slider&)
{
    const float radius  = juce::jmin (width / 2.0f, height / 2.0f) - 4.0f;
    const float cx      = x + width  * 0.5f;
    const float cy      = y + height * 0.5f;
    const float angle   = startAngle + sliderPos * (endAngle - startAngle);

    // ── Outer ring (engraved tick marks) ──────────────────────────────────────
    g.setColour (STEEL.brighter (0.3f));
    for (int i = 0; i <= 10; ++i)
    {
        const float ta  = startAngle + i * (endAngle - startAngle) / 10.0f;
        const float cos = std::cos (ta);
        const float sin = std::sin (ta);
        const float r1  = radius + 5.0f;
        const float r2  = (i == 0 || i == 5 || i == 10) ? radius - 2.0f : radius + 1.0f;
        g.drawLine (cx + r1 * cos, cy + r1 * sin,
                    cx + r2 * cos, cy + r2 * sin, 1.0f);
    }

    // ── Amber arc (position indicator) ────────────────────────────────────────
    {
        juce::Path arc;
        arc.addArc (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f,
                    startAngle, angle, true);
        g.setColour (AMBER.withAlpha (0.85f));
        g.strokePath (arc, juce::PathStrokeType (3.5f,
                       juce::PathStrokeType::curved,
                       juce::PathStrokeType::rounded));
    }

    // ── Bakelite knob body ────────────────────────────────────────────────────
    {
        juce::ColourGradient grad (
            juce::Colour (0xFF2E2A1C), cx - radius * 0.35f, cy - radius * 0.35f,
            juce::Colour (0xFF0E0D09), cx + radius * 0.5f,  cy + radius * 0.5f,
            false);
        g.setGradientFill (grad);
        g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    }

    // ── Top-left sheen ────────────────────────────────────────────────────────
    {
        juce::ColourGradient sheen (
            juce::Colour (0xFF4A4530).withAlpha (0.55f), cx - radius * 0.5f, cy - radius * 0.5f,
            juce::Colour (0xFF000000).withAlpha (0.0f),  cx + radius * 0.4f, cy + radius * 0.4f,
            false);
        g.setGradientFill (sheen);
        g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    }

    // ── Indicator line (cream pointer) ────────────────────────────────────────
    {
        const float lineR = radius * 0.72f;
        g.setColour (CREAM.withAlpha (0.9f));
        g.drawLine (cx, cy,
                    cx + lineR * std::cos (angle),
                    cy + lineR * std::sin (angle),
                    2.5f);
    }

    // ── Centre screw cap ──────────────────────────────────────────────────────
    g.setColour (juce::Colour (0xFF4A4838));
    g.fillEllipse (cx - 4.5f, cy - 4.5f, 9.0f, 9.0f);
    g.setColour (juce::Colour (0xFF7A7660));
    g.drawEllipse (cx - 4.5f, cy - 4.5f, 9.0f, 9.0f, 0.8f);
}

void RetroLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& btn,
                                              const juce::Colour&,
                                              bool isMouseOver, bool isButtonDown)
{
    auto bounds = btn.getLocalBounds().toFloat().reduced (1.0f);
    const bool isToggled = btn.getToggleState();

    // Base fill
    juce::Colour base = isToggled
        ? AMBER.withAlpha (0.85f)
        : juce::Colour (0xFF252318);

    if (isMouseOver)
        base = base.brighter (0.15f);
    if (isButtonDown)
        base = base.darker (0.2f);

    g.setColour (base);
    g.fillRoundedRectangle (bounds, 3.0f);

    // Border
    g.setColour (isToggled ? AMBER.brighter (0.4f) : STEEL.brighter (0.2f));
    g.drawRoundedRectangle (bounds, 3.0f, 1.0f);

    // Inner glow when toggled
    if (isToggled)
    {
        g.setColour (AMBER.withAlpha (0.12f));
        g.fillRoundedRectangle (bounds.reduced (2.0f), 2.0f);
    }
}

void RetroLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& btn,
                                        bool, bool)
{
    const bool isToggled = btn.getToggleState();
    g.setColour (isToggled ? juce::Colour (0xFF0D0D09) : CREAM.withAlpha (0.85f));
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(),
                            11.5f, juce::Font::bold));
    g.drawFittedText (btn.getButtonText(), btn.getLocalBounds(),
                      juce::Justification::centred, 1);
}

void RetroLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height,
                                      bool, int, int, int, int, juce::ComboBox&)
{
    auto bounds = juce::Rectangle<float> (0, 0, (float)width, (float)height);
    g.setColour (PANEL_FACE.brighter (0.1f));
    g.fillRoundedRectangle (bounds, 3.0f);
    g.setColour (STEEL.brighter (0.3f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);

    // Arrow triangle
    const float arrowCX = width - 12.0f;
    const float arrowCY = height * 0.5f;
    juce::Path arrow;
    arrow.addTriangle (arrowCX - 4.0f, arrowCY - 2.0f,
                       arrowCX + 4.0f, arrowCY - 2.0f,
                       arrowCX,        arrowCY + 3.0f);
    g.setColour (AMBER.withAlpha (0.8f));
    g.fillPath (arrow);
}

void RetroLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    g.fillAll (juce::Colour (0xFF151510));
    g.setColour (STEEL);
    g.drawRect (juce::Rectangle<int> (0, 0, w, h), 1);
}

juce::Font RetroLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::plain);
}

void RetroLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& lbl)
{
    g.fillAll (lbl.findColour (juce::Label::backgroundColourId));
    g.setColour (lbl.findColour (juce::Label::textColourId));
    g.setFont   (getLabelFont (lbl));
    g.drawFittedText (lbl.getText(), lbl.getLocalBounds(),
                      lbl.getJustificationType(), 2, 1.0f);
}

juce::Font RetroLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font (juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::bold);
}

//==============================================================================
//  RTLSDRPluginEditor
//==============================================================================

RTLSDRPluginEditor::RTLSDRPluginEditor (RTLSDRPluginProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (860, 600);
    setLookAndFeel (&retroLNF);

    // ── Visualization components ─────────────────────────────────────────────
    addAndMakeVisible (spectrumAnalyzer);
    addAndMakeVisible (waterfallDisplay);
    addAndMakeVisible (signalMeter);

    // ── Knobs ─────────────────────────────────────────────────────────────────
    styleKnob (gainKnob,       "GAIN",       0.0,  50.0, 29.0, " dB");
    styleKnob (volumeKnob,     "VOLUME",     0.0,  1.0,  0.7,  "");
    styleKnob (squelchKnob,    "SQUELCH",    0.0,  1.0,  0.0,  "");
    styleKnob (fineTuneKnob,   "FINE",       -25.0, 25.0, 0.0, " kHz");
    styleKnob (saturationKnob, "SATURATION", 0.0,  1.0,  0.0,  "");

    gainKnob.onValueChange = [this]
    {
        if (audioProcessor.gainParam)
            audioProcessor.gainParam->setValueNotifyingHost (
                audioProcessor.gainParam->getNormalisableRange().convertTo0to1 (
                    (float)gainKnob.getValue()));
    };
    volumeKnob.onValueChange = [this]
    {
        if (audioProcessor.volumeParam)
            audioProcessor.volumeParam->setValueNotifyingHost ((float)volumeKnob.getValue());
    };
    squelchKnob.onValueChange = [this]
    {
        if (audioProcessor.squelchParam)
            audioProcessor.squelchParam->setValueNotifyingHost ((float)squelchKnob.getValue());
    };
    fineTuneKnob.onValueChange = [this]
    {
        if (audioProcessor.fineTuneParam)
            audioProcessor.fineTuneParam->setValueNotifyingHost (
                audioProcessor.fineTuneParam->getNormalisableRange().convertTo0to1 (
                    (float)fineTuneKnob.getValue()));
    };
    saturationKnob.onValueChange = [this]
    {
        if (audioProcessor.saturationParam)
            audioProcessor.saturationParam->setValueNotifyingHost ((float)saturationKnob.getValue());
    };

    for (auto* lbl : { &gainLabel, &volumeLabel, &squelchLabel, &fineTuneLabel, &saturationLabel })
    {
        lbl->setColour (juce::Label::textColourId, RetroLookAndFeel::CREAM);
        lbl->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        lbl->setJustificationType (juce::Justification::centred);
        addAndMakeVisible (lbl);
    }

    // ── AGC button (toggle) ───────────────────────────────────────────────────
    agcButton.setClickingTogglesState (true);
    agcButton.setToggleState (false, juce::dontSendNotification);
    agcButton.onClick = [this]
    {
        if (audioProcessor.agcEnabledParam)
            audioProcessor.agcEnabledParam->setValueNotifyingHost (
                agcButton.getToggleState() ? 1.0f : 0.0f);
    };
    addAndMakeVisible (agcButton);

    // ── Mode buttons ──────────────────────────────────────────────────────────
    auto wireMode = [&](juce::TextButton& btn, int idx)
    {
        btn.setClickingTogglesState (false);
        btn.onClick = [this, idx] { setMode (idx); };
        addAndMakeVisible (btn);
    };
    wireMode (modeWFM, 0);
    wireMode (modeNFM, 1);
    wireMode (modeAM,  2);
    wireMode (modeUSB, 3);
    wireMode (modeLSB, 4);

    // ── Band preset buttons ───────────────────────────────────────────────────
    bandFM.onClick  = [this] { tuneToFrequency (98.7);   };
    bandAir.onClick = [this] { tuneToFrequency (121.5);  };
    bandWx.onClick  = [this] { tuneToFrequency (162.55); };
    bandHam.onClick = [this] { tuneToFrequency (144.2);  };
    bandHF.onClick  = [this] { tuneToFrequency (14.3);   };

    for (auto* b : { &bandFM, &bandAir, &bandWx, &bandHam, &bandHF })
        addAndMakeVisible (b);

    // ── Device & transport ────────────────────────────────────────────────────
    deviceCombo.setTextWhenNothingSelected ("-- SELECT DEVICE --");
    addAndMakeVisible (deviceCombo);

    startButton.onClick = [this]
    {
        const int idx = deviceCombo.getSelectedId() - 2;
        if (idx >= 0)
        {
            if (audioProcessor.startSDR (idx))
            {
                statusLabel.setText ("● ON AIR — " + deviceCombo.getText(),
                                     juce::dontSendNotification);
                statusLabel.setColour (juce::Label::textColourId,
                                       RetroLookAndFeel::AMBER);
            }
            else
            {
                statusLabel.setText ("✗ DEVICE FAULT", juce::dontSendNotification);
                statusLabel.setColour (juce::Label::textColourId,
                                       juce::Colour (0xFFFF3030));
            }
        }
    };
    addAndMakeVisible (startButton);

    stopButton.onClick = [this]
    {
        audioProcessor.stopSDR();
        statusLabel.setText ("■ STANDBY", juce::dontSendNotification);
        statusLabel.setColour (juce::Label::textColourId,
                               RetroLookAndFeel::CREAM.withAlpha (0.5f));
    };
    addAndMakeVisible (stopButton);

    statusLabel.setText ("■ STANDBY", juce::dontSendNotification);
    statusLabel.setColour (juce::Label::textColourId, RetroLookAndFeel::CREAM.withAlpha (0.5f));
    statusLabel.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    statusLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (statusLabel);

    // ── Presets ───────────────────────────────────────────────────────────────
    presetCombo.setTextWhenNothingSelected ("-- PRESETS --");
    presetCombo.onChange = [this]
    {
        const int id = presetCombo.getSelectedId();
        if (id > 0)
            loadPreset (id - 1);
    };
    addAndMakeVisible (presetCombo);

    savePresetButton.onClick   = [this] { saveCurrentAsPreset(); };
    deletePresetButton.onClick = [this] { deleteCurrentPreset(); };
    addAndMakeVisible (savePresetButton);
    addAndMakeVisible (deletePresetButton);

    historyCombo.setTextWhenNothingSelected ("-- HISTORY --");
    historyCombo.onChange = [this]
    {
        const int id = historyCombo.getSelectedId();
        if (id > 0)
        {
            if (auto* entry = frequencyHistory.getEntry (id - 1))
                tuneToFrequency (entry->frequency);
        }
    };
    addAndMakeVisible (historyCombo);

    // ── Click-to-tune from spectrum ───────────────────────────────────────────
    spectrumAnalyzer.onFrequencyClicked = [this](double freq)
    {
        tuneToFrequency (freq);
        addToHistory (freq);
    };

    updateDeviceList();
    updatePresetList();
    updateModeButtons();

    startTimerHz (20);  // 20 Hz UI refresh
}

RTLSDRPluginEditor::~RTLSDRPluginEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void RTLSDRPluginEditor::styleKnob (juce::Slider& s, const juce::String& name,
                                     double lo, double hi, double def,
                                     const juce::String& suffix)
{
    s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    s.setRange (lo, hi);
    s.setValue (def, juce::dontSendNotification);
    s.setDoubleClickReturnValue (true, def);
    if (suffix.isNotEmpty())
        s.setTextValueSuffix (suffix);
    addAndMakeVisible (s);

    // Set corresponding label text (caller has already added label as visible)
    if      (&s == &gainKnob)       gainLabel.setText       (name, juce::dontSendNotification);
    else if (&s == &volumeKnob)     volumeLabel.setText     (name, juce::dontSendNotification);
    else if (&s == &squelchKnob)    squelchLabel.setText    (name, juce::dontSendNotification);
    else if (&s == &fineTuneKnob)   fineTuneLabel.setText   (name, juce::dontSendNotification);
    else if (&s == &saturationKnob) saturationLabel.setText (name, juce::dontSendNotification);
}

//==============================================================================
void RTLSDRPluginEditor::paint (juce::Graphics& g)
{
    using LNF = RetroLookAndFeel;

    // ── Full chassis background ──────────────────────────────────────────────
    g.fillAll (LNF::CHASSIS_BG);

    // ── Panel face (slightly lighter) ─────────────────────────────────────────
    g.setColour (LNF::PANEL_FACE);
    g.fillRect (0, 48, getWidth(), getHeight() - 48);

    // ── Header ────────────────────────────────────────────────────────────────
    drawHeader (g);

    // ── VFO display panel ─────────────────────────────────────────────────────
    {
        auto vfoBounds = juce::Rectangle<int> (0, 48, getWidth() * 58 / 100, 85);
        drawVFODisplay (g, vfoBounds);
    }

    // ── Section separators ────────────────────────────────────────────────────
    // Below VFO/S-meter
    drawEngravedLine (g, 6, 133, getWidth() - 6);
    // Below spectrum
    drawEngravedLine (g, 6, 248, getWidth() - 6);
    // Below waterfall
    drawEngravedLine (g, 6, 338, getWidth() - 6);
    // Below mode row
    drawEngravedLine (g, 6, 386, getWidth() - 6);
    // Below knob row
    drawEngravedLine (g, 6, 496, getWidth() - 6);
    // Below device row
    drawEngravedLine (g, 6, 532, getWidth() - 6);
    // Above footer
    drawEngravedLine (g, 6, 568, getWidth() - 6);

    // ── Section labels ────────────────────────────────────────────────────────
    drawSectionLabel (g, juce::Rectangle<int> (6, 340, 80, 14), "MODE");
    drawSectionLabel (g, juce::Rectangle<int> (getWidth() / 2, 340, 80, 14), "BAND");
    drawSectionLabel (g, juce::Rectangle<int> (6, 388, 60, 14), "CONTROLS");

    // ── S-Meter label ─────────────────────────────────────────────────────────
    g.setColour (LNF::CREAM.withAlpha (0.5f));
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
    g.drawText ("S-METER", getWidth() * 58 / 100 + 4, 50, 70, 12,
                juce::Justification::left);
}

void RTLSDRPluginEditor::drawHeader (juce::Graphics& g)
{
    using LNF = RetroLookAndFeel;

    // Dark steel header
    juce::ColourGradient hdr (
        juce::Colour (0xFF1A1912), 0.0f, 0.0f,
        juce::Colour (0xFF0D0D09), 0.0f, 48.0f, false);
    g.setGradientFill (hdr);
    g.fillRect (0, 0, getWidth(), 48);

    // Bottom border
    g.setColour (LNF::STEEL.brighter (0.5f));
    g.fillRect (0, 47, getWidth(), 1);

    // Corner screws
    drawScrew (g, 20.0f, 24.0f);
    drawScrew (g, (float)getWidth() - 20.0f, 24.0f);

    // ── Title ─────────────────────────────────────────────────────────────────
    g.setColour (LNF::AMBER.withAlpha (0.95f));
    g.setFont   (juce::Font (juce::Font::getDefaultMonospacedFontName(), 20.0f, juce::Font::bold));
    g.drawText  ("RTL·SDR  RADIO  RECEIVER", 40, 8, getWidth() - 180, 24,
                 juce::Justification::left);

    // Model number
    g.setColour  (LNF::CREAM.withAlpha (0.4f));
    g.setFont    (juce::Font (juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::plain));
    g.drawText   ("MODEL SDR-3000  ·  AUDIORF", 40, 30, getWidth() - 180, 14,
                  juce::Justification::left);

    // ── Status LED ────────────────────────────────────────────────────────────
    const bool running = audioProcessor.isSDRRunning();
    const juce::Colour ledCol = running ? LNF::AMBER : juce::Colour (0xFF2A2510);

    // LED halo
    if (running)
    {
        g.setColour (LNF::AMBER.withAlpha (0.15f));
        g.fillEllipse ((float)getWidth() - 68.0f, 14.0f, 28.0f, 28.0f);
    }
    // LED body
    juce::ColourGradient ledGrad (ledCol.brighter (0.4f), (float)getWidth() - 60.0f, 18.0f,
                                   ledCol.darker  (0.3f), (float)getWidth() - 50.0f, 34.0f, false);
    g.setGradientFill (ledGrad);
    g.fillEllipse ((float)getWidth() - 63.0f, 17.0f, 22.0f, 22.0f);
    g.setColour (LNF::STEEL);
    g.drawEllipse ((float)getWidth() - 63.0f, 17.0f, 22.0f, 22.0f, 1.0f);

    // LED label
    g.setColour  (LNF::CREAM.withAlpha (0.5f));
    g.setFont    (juce::Font (juce::Font::getDefaultMonospacedFontName(), 8.0f, juce::Font::bold));
    g.drawText   (running ? "ON AIR" : "STDBY", getWidth() - 100, 30, 95, 12,
                  juce::Justification::right);
}

//==============================================================================
// 7-Segment display helpers

static const uint8_t SEG_MAP[] =
{
    0b0111111,  // '0'  a b c d e f
    0b0000110,  // '1'  b c
    0b1011011,  // '2'  a b d e g
    0b1001111,  // '3'  a b c d g
    0b1100110,  // '4'  b c f g
    0b1101101,  // '5'  a c d f g
    0b1111101,  // '6'  a c d e f g
    0b0000111,  // '7'  a b c
    0b1111111,  // '8'  all
    0b1101111,  // '9'  a b c d f g
};

void RTLSDRPluginEditor::drawSevenSegDigit (juce::Graphics& g,
                                             float x, float y, float w, float h, char c)
{
    using LNF = RetroLookAndFeel;

    uint8_t segs = 0;
    if (c >= '0' && c <= '9')
        segs = SEG_MAP[c - '0'];
    else if (c == '-')
        segs = 0b1000000;

    const float sw   = w * 0.12f;   // segment thickness
    const float gap  = 1.0f;         // inter-segment gap

    auto seg = [&](bool on, float sx, float sy, float sw2, float sh)
    {
        g.setColour (on ? LNF::AMBER : LNF::AMBER_DIM);
        g.fillRect (sx + gap, sy + gap, sw2 - gap * 2.0f, sh - gap * 2.0f);
    };

    // a: top
    seg (segs & 1,  x + sw,       y,              w - sw * 2.0f, sw);
    // b: top-right
    seg (segs & 2,  x + w - sw,   y + sw,         sw,            h * 0.5f - sw);
    // c: bot-right
    seg (segs & 4,  x + w - sw,   y + h * 0.5f,   sw,            h * 0.5f - sw);
    // d: bottom
    seg (segs & 8,  x + sw,       y + h - sw,     w - sw * 2.0f, sw);
    // e: bot-left
    seg (segs & 16, x,             y + h * 0.5f,   sw,            h * 0.5f - sw);
    // f: top-left
    seg (segs & 32, x,             y + sw,         sw,            h * 0.5f - sw);
    // g: middle
    seg (segs & 64, x + sw,       y + h * 0.5f - sw * 0.5f, w - sw * 2.0f, sw);
}

void RTLSDRPluginEditor::drawVFODisplay (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    using LNF = RetroLookAndFeel;

    // Display bezel
    g.setColour (LNF::STEEL.darker (0.3f));
    g.fillRoundedRectangle (bounds.toFloat().reduced (4.0f), 4.0f);

    // Inner display face
    auto face = bounds.reduced (8);
    g.setColour (LNF::DISPLAY_BG);
    g.fillRoundedRectangle (face.toFloat(), 3.0f);

    // Phosphor glow layer
    juce::ColourGradient glow (
        LNF::AMBER.withAlpha (0.04f),     face.getCentreX(), face.getY(),
        juce::Colours::transparentBlack,  face.getCentreX(), face.getBottom(), false);
    g.setGradientFill (glow);
    g.fillRoundedRectangle (face.toFloat(), 3.0f);

    // ── Format frequency string ────────────────────────────────────────────────
    const double freqMHz = displayedFreqMHz;
    const int    wholeMHz = (int)freqMHz;
    const int    fracKHz  = (int)std::round ((freqMHz - wholeMHz) * 1000.0);

    // Format: "XXXX.XXX" or " XXX.XXX"
    const juce::String freqStr = juce::String::formatted ("%4d.%03d", wholeMHz, fracKHz);

    // ── Draw 7-segment digits ─────────────────────────────────────────────────
    const float digitH  = face.getHeight() * 0.70f;
    const float digitW  = digitH * 0.58f;
    const float dotW    = digitW * 0.25f;
    const float spacing = digitW + 4.0f;
    const int   numChars = (int)freqStr.length(); // should be 8 (4 + dot + 3)

    // Total width for all chars
    // We have 4 integer digits, 1 dot, 3 fractional digits
    const float totalW = 4.0f * spacing + dotW + 3.0f * spacing;
    float startX = face.getX() + (face.getWidth() - totalW) * 0.5f;
    const float startY = face.getY() + (face.getHeight() - digitH) * 0.5f;

    for (int i = 0; i < numChars; ++i)
    {
        const char c = freqStr[i];
        if (c == '.')
        {
            // Decimal point — small amber dot
            g.setColour (LNF::AMBER);
            g.fillEllipse (startX + 1.0f, startY + digitH - dotW - 2.0f, dotW, dotW);
            startX += dotW + 4.0f;
        }
        else
        {
            drawSevenSegDigit (g, startX, startY, digitW, digitH, c);
            startX += spacing;
        }
    }

    // ── "MHz" label ───────────────────────────────────────────────────────────
    g.setColour  (LNF::AMBER.withAlpha (0.6f));
    g.setFont    (juce::Font (juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold));
    g.drawText   ("MHz", face.getRight() - 38, face.getBottom() - 18, 35, 14,
                  juce::Justification::right);
}

void RTLSDRPluginEditor::drawEngravedLine (juce::Graphics& g, int x1, int y, int x2)
{
    g.setColour (juce::Colour (0xFF0A0A06));
    g.drawLine  ((float)x1, (float)y, (float)x2, (float)y, 1.0f);
    g.setColour (juce::Colour (0xFF2E2C20));
    g.drawLine  ((float)x1, (float)y + 1.0f, (float)x2, (float)y + 1.0f, 0.8f);
}

void RTLSDRPluginEditor::drawSectionLabel (juce::Graphics& g,
                                            juce::Rectangle<int> r, const juce::String& text)
{
    g.setColour (RetroLookAndFeel::CREAM.withAlpha (0.38f));
    g.setFont   (juce::Font (juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
    g.drawText  (text, r, juce::Justification::left);
}

void RTLSDRPluginEditor::drawScrew (juce::Graphics& g, float cx, float cy)
{
    g.setColour (juce::Colour (0xFF3A3825));
    g.fillEllipse (cx - 7.0f, cy - 7.0f, 14.0f, 14.0f);
    g.setColour (juce::Colour (0xFF555340));
    g.drawEllipse (cx - 7.0f, cy - 7.0f, 14.0f, 14.0f, 1.0f);
    // Slot
    g.setColour (juce::Colour (0xFF1A1912));
    g.drawLine  (cx - 4.0f, cy, cx + 4.0f, cy, 1.5f);
}

//==============================================================================
void RTLSDRPluginEditor::resized()
{
    const int W = getWidth();

    // ── Spectrum analyzer ─────────────────────────────────────────────────────
    spectrumAnalyzer.setBounds (4, 135, W - 8, 111);

    // ── Waterfall ─────────────────────────────────────────────────────────────
    waterfallDisplay.setBounds (4, 250, W - 8, 86);

    // ── S-Meter (right of VFO) ────────────────────────────────────────────────
    const int vfoW = W * 58 / 100;
    signalMeter.setBounds (vfoW + 2, 64, W - vfoW - 6, 60);

    // ── Mode buttons (left half of mode row) ──────────────────────────────────
    {
        const int btnY  = 354;
        const int btnH  = 26;
        const int btnW  = 62;
        const int gapX  = 4;
        int bx = 10;
        for (auto* b : { &modeWFM, &modeNFM, &modeAM, &modeUSB, &modeLSB })
        {
            b->setBounds (bx, btnY, btnW, btnH);
            bx += btnW + gapX;
        }
    }

    // ── Band buttons (right half of mode row) ─────────────────────────────────
    {
        const int btnY  = 354;
        const int btnH  = 26;
        const int btnW  = 55;
        const int gapX  = 4;
        int bx = W / 2 + 4;
        for (auto* b : { &bandFM, &bandAir, &bandWx, &bandHam, &bandHF })
        {
            b->setBounds (bx, btnY, btnW, btnH);
            bx += btnW + gapX;
        }
    }

    // ── Knob row ──────────────────────────────────────────────────────────────
    {
        const int knobY  = 398;
        const int knobSz = 72;
        const int lblH   = 14;
        const int rowH   = knobSz + lblH + 4;
        juce::ignoreUnused (rowH);

        // Distribute 5 knobs + AGC button evenly
        const int knobSpacing = (W - 20) / 6;
        int kx = 10 + knobSpacing / 2 - knobSz / 2;

        auto placeKnob = [&](juce::Slider& s, juce::Label& lbl)
        {
            s.setBounds   (kx, knobY, knobSz, knobSz);
            lbl.setBounds (kx - 6, knobY + knobSz + 2, knobSz + 12, lblH);
            kx += knobSpacing;
        };

        placeKnob (gainKnob,       gainLabel);
        placeKnob (volumeKnob,     volumeLabel);
        placeKnob (squelchKnob,    squelchLabel);
        placeKnob (fineTuneKnob,   fineTuneLabel);
        placeKnob (saturationKnob, saturationLabel);

        // AGC button — same row
        agcButton.setBounds (kx, knobY + 20, knobSpacing - 10, 34);
    }

    // ── Device / transport row ────────────────────────────────────────────────
    {
        const int rowY = 499;
        const int rowH = 28;
        deviceCombo.setBounds   (8,         rowY, 340, rowH);
        startButton.setBounds   (356,       rowY, 130, rowH);
        stopButton.setBounds    (492,       rowY, 100, rowH);
        statusLabel.setBounds   (600,       rowY, W - 608, rowH);
    }

    // ── Preset / history row ──────────────────────────────────────────────────
    {
        const int rowY = 535;
        const int rowH = 28;
        presetCombo.setBounds       (8,   rowY, 280, rowH);
        savePresetButton.setBounds  (295, rowY, 80,  rowH);
        deletePresetButton.setBounds(381, rowY, 80,  rowH);
        historyCombo.setBounds      (470, rowY, 200, rowH);
    }
}

//==============================================================================
void RTLSDRPluginEditor::timerCallback()
{
    updateStatus();

    // ── Mirror parameters → controls (handle automation / remote changes) ─────
    if (audioProcessor.gainParam)
        gainKnob.setValue (audioProcessor.gainParam->get(), juce::dontSendNotification);
    if (audioProcessor.volumeParam)
        volumeKnob.setValue (audioProcessor.volumeParam->get(), juce::dontSendNotification);
    if (audioProcessor.squelchParam)
        squelchKnob.setValue (audioProcessor.squelchParam->get(), juce::dontSendNotification);
    if (audioProcessor.fineTuneParam)
        fineTuneKnob.setValue (audioProcessor.fineTuneParam->get(), juce::dontSendNotification);
    if (audioProcessor.saturationParam)
        saturationKnob.setValue (audioProcessor.saturationParam->get(), juce::dontSendNotification);
    if (audioProcessor.agcEnabledParam)
        agcButton.setToggleState (audioProcessor.agcEnabledParam->get() > 0.5f,
                                  juce::dontSendNotification);

    // Update displayed frequency (freq + fine tune)
    if (audioProcessor.frequencyParam)
    {
        double freq = audioProcessor.frequencyParam->get();
        if (audioProcessor.fineTuneParam)
            freq += audioProcessor.fineTuneParam->get() * 0.001;  // kHz → MHz
        if (std::abs (freq - displayedFreqMHz) > 0.0001)
        {
            displayedFreqMHz = freq;
            repaint (0, 48, getWidth() * 58 / 100, 85);  // repaint VFO only
        }
    }

    // ── Feed IQ data to visualizers ───────────────────────────────────────────
    if (audioProcessor.isSDRRunning())
    {
        auto& iq = audioProcessor.getIQBuffer();
        uint8_t buf[2048];
        const int got = iq.read (buf, 2048);
        if (got > 0)
        {
            spectrumAnalyzer.pushSamples (buf, got);
            waterfallDisplay.pushSamples (buf, got);
            signalMeter.updateSignal     (buf, got);
        }
    }
}

//==============================================================================
void RTLSDRPluginEditor::updateStatus()
{
    // Just repaint the LED area in the header
    repaint (getWidth() - 110, 0, 110, 48);
}

void RTLSDRPluginEditor::setMode (int idx)
{
    currentModeIndex = idx;
    if (audioProcessor.modeParam)
        audioProcessor.modeParam->setValueNotifyingHost (
            (float)idx / (float)(audioProcessor.modeParam->choices.size() - 1));
    updateModeButtons();
}

void RTLSDRPluginEditor::updateModeButtons()
{
    int idx = audioProcessor.modeParam ? audioProcessor.modeParam->getIndex() : currentModeIndex;
    modeWFM.setToggleState (idx == 0, juce::dontSendNotification);
    modeNFM.setToggleState (idx == 1, juce::dontSendNotification);
    modeAM .setToggleState (idx == 2, juce::dontSendNotification);
    modeUSB.setToggleState (idx == 3, juce::dontSendNotification);
    modeLSB.setToggleState (idx == 4, juce::dontSendNotification);
}

void RTLSDRPluginEditor::tuneToFrequency (double mhz)
{
    if (audioProcessor.frequencyParam)
    {
        audioProcessor.frequencyParam->setValueNotifyingHost (
            audioProcessor.frequencyParam->getNormalisableRange().convertTo0to1 ((float)mhz));
        spectrumAnalyzer.setCenterFrequency (mhz * 1e6);
        waterfallDisplay.setCenterFrequency (mhz * 1e6);
    }
    addToHistory (mhz);
}

void RTLSDRPluginEditor::updateDeviceList()
{
    deviceCombo.clear();
    auto devices = audioProcessor.getAvailableDevices();
    if (devices.empty())
    {
        deviceCombo.addItem ("No RTL-SDR devices found", 1);
        deviceCombo.setSelectedId (1);
    }
    else
    {
        for (size_t i = 0; i < devices.size(); ++i)
            deviceCombo.addItem (devices[i].name + "  (" + devices[i].serial + ")", (int)i + 2);
        deviceCombo.setSelectedId (2);
    }
}

void RTLSDRPluginEditor::updatePresetList()
{
    presetCombo.clear();
    for (int i = 0; i < presetManager.getNumPresets(); ++i)
        presetCombo.addItem (presetManager.getPresets()[i].name, i + 1);
}

void RTLSDRPluginEditor::loadPreset (int index)
{
    if (index < 0 || index >= presetManager.getNumPresets()) return;
    const auto& p = presetManager.getPreset (index);

    tuneToFrequency (p.frequency);
    gainKnob.setValue (p.gain, juce::sendNotification);

    if      (p.mode == "FM") setMode (0);
    else if (p.mode == "AM") setMode (2);
}

void RTLSDRPluginEditor::saveCurrentAsPreset()
{
    juce::AlertWindow::showAsync (
        juce::MessageBoxOptions()
            .withIconType  (juce::MessageBoxIconType::QuestionIcon)
            .withTitle     ("Save Preset")
            .withMessage   ("Save current frequency as preset?")
            .withButton    ("OK")
            .withButton    ("Cancel"),
        [this](int result)
        {
            if (result == 1)
            {
                RadioPreset p;
                p.name      = "Preset " + juce::String (presetManager.getNumPresets() + 1);
                p.frequency = displayedFreqMHz;
                p.gain      = (float)gainKnob.getValue();
                p.mode      = (currentModeIndex == 2) ? "AM" : "FM";
                presetManager.addPreset (p);
                updatePresetList();
            }
        });
}

void RTLSDRPluginEditor::deleteCurrentPreset()
{
    const int id = presetCombo.getSelectedId();
    if (id > 0)
    {
        presetManager.removePreset (id - 1);
        updatePresetList();
        presetCombo.setSelectedId (0);
    }
}

void RTLSDRPluginEditor::updateHistoryList()
{
    historyCombo.clear();
    const auto& history = frequencyHistory.getHistory();
    int id = 1;
    for (const auto& entry : history)
    {
        juce::String text = juce::String (entry.frequency, 3) + " MHz";
        if (entry.label.isNotEmpty())
            text += "  " + entry.label;
        historyCombo.addItem (text, id++);
    }
}

void RTLSDRPluginEditor::addToHistory (double mhz)
{
    frequencyHistory.addFrequency (mhz);
    updateHistoryList();
}
