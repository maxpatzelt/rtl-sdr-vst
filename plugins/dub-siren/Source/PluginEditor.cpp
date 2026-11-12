#include "PluginEditor.h"
#include <BinaryData.h>

// ============================================================================
// DubLookAndFeel
// ============================================================================

DubLookAndFeel::DubLookAndFeel()
{
    // Global colour overrides so JUCE doesn't paint over our custom work
    setColour(juce::Slider::rotarySliderFillColourId,    juce::Colour(AMBER));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(AMBER_DIM));
    setColour(juce::Label::textColourId,                 juce::Colour(CREAM));
    setColour(juce::ComboBox::backgroundColourId,        juce::Colour(0x88141210));
    setColour(juce::ComboBox::textColourId,              juce::Colour(AMBER));
    setColour(juce::ComboBox::outlineColourId,           juce::Colour(0xFF3A3020));
    setColour(juce::ComboBox::arrowColourId,             juce::Colour(AMBER));
    setColour(juce::PopupMenu::backgroundColourId,       juce::Colour(0xFF1A1710));
    setColour(juce::PopupMenu::textColourId,             juce::Colour(CREAM));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF3A2800));
    setColour(juce::PopupMenu::highlightedTextColourId,  juce::Colour(AMBER));
}

void DubLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                      int x, int y, int width, int height,
                                      float sliderPos,
                                      float startAngle, float endAngle,
                                      juce::Slider& /*slider*/)
{
    const float cx     = x + width  * 0.5f;
    const float cy     = y + height * 0.5f;
    const float radius = juce::jmin(width, height) * 0.5f - 5.0f;
    const float angle  = startAngle + sliderPos * (endAngle - startAngle);

    // ── Track ring ────────────────────────────────────────────────────────
    const float trackW = 3.5f;
    {
        juce::Path track;
        track.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, endAngle, true);
        g.setColour(juce::Colour(AMBER_DIM));
        g.strokePath(track, juce::PathStrokeType(trackW, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    // ── Filled arc (progress) ─────────────────────────────────────────────
    {
        juce::Path arc;
        arc.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, angle, true);
        g.setColour(juce::Colour(AMBER));
        g.strokePath(arc, juce::PathStrokeType(trackW, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    // ── Knob body (bakelite-style radial gradient) ────────────────────────
    const float kr = radius - trackW - 2.0f;
    {
        juce::ColourGradient bodyGrad(
            juce::Colour(0xFF3A3020), cx - kr * 0.35f, cy - kr * 0.35f,
            juce::Colour(KNOB_DARK),  cx + kr * 0.35f, cy + kr * 0.35f,
            true);
        bodyGrad.addColour(0.5, juce::Colour(0xFF2A2418));
        g.setGradientFill(bodyGrad);
        g.fillEllipse(cx - kr, cy - kr, kr * 2.0f, kr * 2.0f);
    }

    // ── Rim highlight ─────────────────────────────────────────────────────
    g.setColour(juce::Colour(KNOB_RIM));
    g.drawEllipse(cx - kr, cy - kr, kr * 2.0f, kr * 2.0f, 1.2f);

    // Top-left sheen
    g.setColour(juce::Colour(0x22FFFFFF));
    g.drawEllipse(cx - kr + 1.5f, cy - kr + 1.5f, kr * 0.9f, kr * 0.9f, 0.8f);

    // ── Cream indicator line ──────────────────────────────────────────────
    {
        const float lineStart = kr * 0.25f;
        const float lineEnd   = kr * 0.85f;
        const float sinA      = std::sin(angle);
        const float cosA      = std::cos(angle);
        g.setColour(juce::Colour(CREAM).withAlpha(0.9f));
        g.drawLine(cx + sinA * lineStart, cy - cosA * lineStart,
                   cx + sinA * lineEnd,   cy - cosA * lineEnd,   1.8f);

        // Bright tip dot
        g.setColour(juce::Colour(AMBER).withAlpha(0.8f));
        g.fillEllipse(cx + sinA * (lineEnd - 1.0f) - 2.0f,
                      cy - cosA * (lineEnd - 1.0f) - 2.0f, 4.0f, 4.0f);
    }

    // ── Centre screw cap ─────────────────────────────────────────────────
    const float screwR = kr * 0.18f;
    g.setColour(juce::Colour(0xFF181410));
    g.fillEllipse(cx - screwR, cy - screwR, screwR * 2.0f, screwR * 2.0f);
    g.setColour(juce::Colour(0xFF403820));
    g.drawEllipse(cx - screwR, cy - screwR, screwR * 2.0f, screwR * 2.0f, 0.8f);
    // Single screw slot
    const float slotAngle = angle;
    g.setColour(juce::Colour(0xFF504428));
    g.drawLine(cx + std::sin(slotAngle) * (-screwR * 0.7f),
               cy - std::cos(slotAngle) * (-screwR * 0.7f),
               cx + std::sin(slotAngle) * ( screwR * 0.7f),
               cy - std::cos(slotAngle) * ( screwR * 0.7f), 1.0f);
}

void DubLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                  bool /*isButtonDown*/,
                                  int /*buttonX*/, int /*buttonY*/,
                                  int /*buttonW*/, int /*buttonH*/,
                                  juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height);

    g.setColour(juce::Colour(0x88141210));
    g.fillRoundedRectangle(bounds, 3.0f);

    g.setColour(juce::Colour(0xAA4A3A18));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

    // Arrow
    const float arrowX = width - 14.0f;
    const float arrowY = height * 0.5f;
    juce::Path arrow;
    arrow.startNewSubPath(arrowX,        arrowY - 3.0f);
    arrow.lineTo         (arrowX + 6.0f, arrowY - 3.0f);
    arrow.lineTo         (arrowX + 3.0f, arrowY + 3.0f);
    arrow.closeSubPath();
    g.setColour(juce::Colour(AMBER).withAlpha(0.7f));
    g.fillPath(arrow);
}

void DubLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(label.getFont());
    g.drawText(label.getText(), label.getLocalBounds(), label.getJustificationType(), false);
}

void DubLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                          const juce::Colour&,
                                          bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

    // Body gradient — brighter when held
    juce::ColourGradient body(
        juce::Colour(isButtonDown ? 0xFF4A3200 : 0xFF1E1810), bounds.getX(), bounds.getY(),
        juce::Colour(isButtonDown ? 0xFF261800 : 0xFF0E0C08), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(body);
    g.fillRoundedRectangle(bounds, 3.0f);

    // Border — amber when held, dim when idle
    const auto borderColour = isButtonDown  ? juce::Colour(0xFFFFB300)
                            : isMouseOverButton ? juce::Colour(0xFF705030)
                                               : juce::Colour(0xFF3A2800);
    g.setColour(borderColour);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.2f);

    // Amber glow halo when held
    if (isButtonDown)
    {
        g.setColour(juce::Colour(0x28FFB300));
        g.fillRoundedRectangle(bounds.expanded(3.0f), 6.0f);
    }
}

void DubLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                    bool /*isMouseOver*/, bool isButtonDown)
{
    g.setColour(juce::Colour(isButtonDown ? 0xFFFFB300 : 0xFF806040));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), button.getLocalBounds(),
               juce::Justification::centred, false);
}

// ============================================================================
// Helper — adds a rotary knob + label to the editor
// ============================================================================
static void setupKnob(juce::Slider& s)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

static void setupLabel(juce::Label& lbl, const juce::String& text)
{
    lbl.setText(text, juce::dontSendNotification);
    lbl.setJustificationType(juce::Justification::centred);
    lbl.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::plain));
    lbl.setColour(juce::Label::textColourId, juce::Colour(0xFFCFC9A8).withAlpha(0.8f));
}

// ============================================================================
// DubSirenEditor
// ============================================================================
DubSirenEditor::DubSirenEditor(DubSirenProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    auto& params = processorRef.getParameters();

    // ── Apply our LookAndFeel globally ─────────────────────────────────────
    setLookAndFeel(&dubLAF);

    // ── Trigger button ──────────────────────────────────────────────────────
    triggerButton.setButtonText("TRIGGER");
    triggerButton.onStateChange = [this]()
    {
        const bool down = (triggerButton.getState() == juce::Button::buttonDown);
        if (down && !buttonIsDown_)
        {
            processorRef.triggerButtonOn();
            buttonIsDown_ = true;
        }
        else if (!down && buttonIsDown_)
        {
            processorRef.triggerButtonOff();
            buttonIsDown_ = false;
        }
    };
    addChildComponent(triggerButton);  // kept but hidden — MIDI keyboard replaces it

    // ── Sample selector + button ────────────────────────────────────────────
    for (int i = 0; i < processorRef.getNumSamples(); ++i)
        sampleSelectBox.addItem("SHOT " + juce::String(i + 1), i + 1);
    sampleSelectBox.setSelectedId(1, juce::dontSendNotification);
    sampleSelectBox.onChange = [this]()
    {
        processorRef.selectSample(sampleSelectBox.getSelectedId() - 1);
    };
    addAndMakeVisible(sampleSelectBox);

    sampleButton.setButtonText("FIRE");
    sampleButton.onClick = [this]() { processorRef.sampleButtonTrigger(); };
    addAndMakeVisible(sampleButton);

    // ── MIDI target selector ────────────────────────────────────────────────
    midiTargetBox.addItem("VCO",    1);
    midiTargetBox.addItem("SAMPLE", 2);
    midiTargetBox.addItem("BOTH",   3);
    midiTargetAttach = std::make_unique<ChoiceAttach>(params, "midiTarget", midiTargetBox);
    addAndMakeVisible(midiTargetBox);

    // ── Randomize button ────────────────────────────────────────────────────
    randomizeButton.setButtonText("RANDOMIZE");
    randomizeButton.onClick = [this]()
    {
        juce::Random rng;
        for (auto* param : static_cast<juce::AudioProcessor&>(processorRef).getParameters())
            param->setValueNotifyingHost(rng.nextFloat());
        // Also pick a random sample and sync the UI selector
        const int randomSample = rng.nextInt(processorRef.getNumSamples());
        processorRef.selectSample(randomSample);
        sampleSelectBox.setSelectedId(randomSample + 1, juce::sendNotification);
    };
    addAndMakeVisible(randomizeButton);

    // ── VCO ────────────────────────────────────────────────────────────────
    setupKnob(vcoRateSlider);    addAndMakeVisible(vcoRateSlider);
    setupKnob(vcoLevelSlider);   addAndMakeVisible(vcoLevelSlider);
    setupKnob(portamentoSlider); addAndMakeVisible(portamentoSlider);

    setupLabel(vcoRateLabel,    "SIREN");
    setupLabel(vcoLevelLabel,   "LEVEL");
    setupLabel(portamentoLabel, "PORTA");
    setupLabel(vcoWaveLabel,    "WAVE");
    addAndMakeVisible(vcoRateLabel);
    addAndMakeVisible(vcoLevelLabel);
    addAndMakeVisible(portamentoLabel);
    addAndMakeVisible(vcoWaveLabel);

    vcoWaveformBox.addItem("Square", 1);
    vcoWaveformBox.addItem("Saw",    2);
    vcoWaveformBox.addItem("Tri",    3);
    addAndMakeVisible(vcoWaveformBox);

    // ── Delay ──────────────────────────────────────────────────────────────
    setupKnob(delayTimeSlider);     addAndMakeVisible(delayTimeSlider);
    setupKnob(delayFeedbackSlider); addAndMakeVisible(delayFeedbackSlider);
    setupKnob(delayWetDrySlider);   addAndMakeVisible(delayWetDrySlider);

    setupLabel(delayTimeLabel,     "ECHO");
    setupLabel(delayFeedbackLabel, "REPEATS");
    setupLabel(delayWetDryLabel,   "DUB MIX");
    addAndMakeVisible(delayTimeLabel);
    addAndMakeVisible(delayFeedbackLabel);
    addAndMakeVisible(delayWetDryLabel);

    // ── LFO 1 ──────────────────────────────────────────────────────────────
    setupKnob(lfo1RateSlider);   addAndMakeVisible(lfo1RateSlider);
    setupKnob(lfo1AmountSlider); addAndMakeVisible(lfo1AmountSlider);

    setupLabel(lfo1RateLabel,   "WOBBLE");
    setupLabel(lfo1AmountLabel, "DEPTH");
    setupLabel(lfo1TargetLabel, "→ TARGET");
    setupLabel(lfo1WaveLabel,   "SHAPE");
    addAndMakeVisible(lfo1RateLabel);
    addAndMakeVisible(lfo1AmountLabel);
    addAndMakeVisible(lfo1TargetLabel);
    addAndMakeVisible(lfo1WaveLabel);

    lfo1TargetBox.addItem("None",        1);
    lfo1TargetBox.addItem("VCO Rate",    2);
    lfo1TargetBox.addItem("Delay",       3);
    lfo1TargetBox.addItem("Sample Rate", 4);
    addAndMakeVisible(lfo1TargetBox);

    lfo1WaveformBox.addItem("Sine",   1);
    lfo1WaveformBox.addItem("Tri",    2);
    lfo1WaveformBox.addItem("Square", 3);
    lfo1WaveformBox.addItem("S+H",    4);
    addAndMakeVisible(lfo1WaveformBox);

    // ── LFO 2 ──────────────────────────────────────────────────────────────
    setupKnob(lfo2RateSlider);   addAndMakeVisible(lfo2RateSlider);
    setupKnob(lfo2AmountSlider); addAndMakeVisible(lfo2AmountSlider);

    setupLabel(lfo2RateLabel,   "SHAKE");
    setupLabel(lfo2AmountLabel, "POWER");
    setupLabel(lfo2TargetLabel, "→ TARGET");
    setupLabel(lfo2WaveLabel,   "SHAPE");
    addAndMakeVisible(lfo2RateLabel);
    addAndMakeVisible(lfo2AmountLabel);
    addAndMakeVisible(lfo2TargetLabel);
    addAndMakeVisible(lfo2WaveLabel);

    lfo2TargetBox.addItem("None",        1);
    lfo2TargetBox.addItem("LFO1 Rate",   2);
    lfo2TargetBox.addItem("LFO1 Amount", 3);
    lfo2TargetBox.addItem("Delay",       4);
    lfo2TargetBox.addItem("Sample Rate", 5);
    addAndMakeVisible(lfo2TargetBox);

    lfo2WaveformBox.addItem("Sine",   1);
    lfo2WaveformBox.addItem("Tri",    2);
    lfo2WaveformBox.addItem("Square", 3);
    lfo2WaveformBox.addItem("S+H",    4);
    addAndMakeVisible(lfo2WaveformBox);

    // ── Sample ──────────────────────────────────────────────────────────────
    setupKnob(sampleLevelSlider); addAndMakeVisible(sampleLevelSlider);
    setupKnob(sampleRateSlider);  addAndMakeVisible(sampleRateSlider);
    setupLabel(sampleLevelLabel, "SMP LVL"); addAndMakeVisible(sampleLevelLabel);
    setupLabel(sampleRateLabel,  "SMP SPD"); addAndMakeVisible(sampleRateLabel);

    // ── APVTS Attachments ──────────────────────────────────────────────────
    vcoRateAttach     = std::make_unique<SliderAttach>(params, "vcoRate",       vcoRateSlider);
    vcoLevelAttach    = std::make_unique<SliderAttach>(params, "vcoLevel",      vcoLevelSlider);
    portamentoAttach  = std::make_unique<SliderAttach>(params, "portamento",    portamentoSlider);
    vcoWaveAttach     = std::make_unique<ChoiceAttach>(params, "vcoWaveform",   vcoWaveformBox);

    delayTimeAttach    = std::make_unique<SliderAttach>(params, "delayTime",     delayTimeSlider);
    delayFeedbackAttach= std::make_unique<SliderAttach>(params, "delayFeedback", delayFeedbackSlider);
    delayWetDryAttach  = std::make_unique<SliderAttach>(params, "delayWetDry",   delayWetDrySlider);

    lfo1RateAttach   = std::make_unique<SliderAttach>(params, "lfo1Rate",     lfo1RateSlider);
    lfo1AmountAttach = std::make_unique<SliderAttach>(params, "lfo1Amount",   lfo1AmountSlider);
    lfo1TargetAttach = std::make_unique<ChoiceAttach>(params, "lfo1Target",   lfo1TargetBox);
    lfo1WaveAttach   = std::make_unique<ChoiceAttach>(params, "lfo1Waveform", lfo1WaveformBox);

    lfo2RateAttach   = std::make_unique<SliderAttach>(params, "lfo2Rate",     lfo2RateSlider);
    lfo2AmountAttach = std::make_unique<SliderAttach>(params, "lfo2Amount",   lfo2AmountSlider);
    lfo2TargetAttach = std::make_unique<ChoiceAttach>(params, "lfo2Target",   lfo2TargetBox);
    lfo2WaveAttach   = std::make_unique<ChoiceAttach>(params, "lfo2Waveform", lfo2WaveformBox);

    sampleLevelAttach = std::make_unique<SliderAttach>(params, "sampleLevel",    sampleLevelSlider);
    sampleRateAttach  = std::make_unique<SliderAttach>(params, "samplePlayRate", sampleRateSlider);

    // ── Filter ─────────────────────────────────────────────────────────────
    setupKnob(filterCutoffSlider);    addAndMakeVisible(filterCutoffSlider);
    setupKnob(filterResonanceSlider); addAndMakeVisible(filterResonanceSlider);
    setupKnob(filterDecaySlider);     addAndMakeVisible(filterDecaySlider);
    setupLabel(filterCutoffLabel,    "SWEEP");
    setupLabel(filterResonanceLabel, "RESO");
    setupLabel(filterDecayLabel,     "DECAY");
    addAndMakeVisible(filterCutoffLabel);
    addAndMakeVisible(filterResonanceLabel);
    addAndMakeVisible(filterDecayLabel);
    filterCutoffAttach    = std::make_unique<SliderAttach>(params, "filterCutoff",     filterCutoffSlider);
    filterResonanceAttach = std::make_unique<SliderAttach>(params, "filterResonance",  filterResonanceSlider);
    filterDecayAttach     = std::make_unique<SliderAttach>(params, "filterDecay",      filterDecaySlider);

    setSize(720, 490);
}

DubSirenEditor::~DubSirenEditor()
{
    setLookAndFeel(nullptr);
}

// ── paint ─────────────────────────────────────────────────────────────────────
void DubSirenEditor::drawSection(juce::Graphics& g,
                                 juce::Rectangle<int> bounds,
                                 const juce::String& title) const
{
    // Section fill — semi-transparent so background image shows through
    g.setColour(juce::Colour(0x401E1B16));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);

    // Engraved border — amber tinted
    g.setColour(juce::Colour(0xBB4A3A18));
    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 4.0f, 1.2f);

    // Section title — engraved look (shadow below, highlight above)
    if (title.isNotEmpty())
    {
        juce::Font f(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold);
        const int tx = bounds.getX() + 10;
        const int ty = bounds.getY() + 6;

        g.setFont(f);
        g.setColour(juce::Colour(0xFF0A0804));
        g.drawText(title, tx + 0, ty + 1, bounds.getWidth() - 12, 12,
                   juce::Justification::left, false);
        g.setColour(juce::Colour(0xFFFFB300).withAlpha(0.55f));
        g.drawText(title, tx, ty, bounds.getWidth() - 12, 12,
                   juce::Justification::left, false);

        // Tiny divider line under title
        g.setColour(juce::Colour(0xFF2E2A1E));
        g.drawHorizontalLine(ty + 14, (float)(bounds.getX() + 8), (float)(bounds.getRight() - 8));
    }
}

void DubSirenEditor::paint(juce::Graphics& g)
{
    const int W = getWidth();
    const int H = getHeight();

    // ── Background image ──────────────────────────────────────────────────
    {
        auto img = juce::ImageCache::getFromMemory(
            BinaryData::background_jfif, BinaryData::background_jfifSize);
        if (img.isValid())
        {
            g.drawImage(img, 0, 0, W, H, 0, 0, img.getWidth(), img.getHeight());
            // Dark overlay to keep UI legible over the photo
            g.setColour(juce::Colour(0xBB0E0C0A));
            g.fillAll();
        }
        else
        {
            juce::ColourGradient bg(
                juce::Colour(0xFF161210), 0.0f,    0.0f,
                juce::Colour(0xFF0E0C0A), (float)W, (float)H, false);
            g.setGradientFill(bg);
            g.fillAll();
        }
    }

    // ── Subtle scan-line texture ──────────────────────────────────────────
    g.setColour(juce::Colour(0x07000000));
    for (int y = 0; y < H; y += 2)
        g.drawHorizontalLine(y, 0.0f, (float)W);

    // ── Header bar ───────────────────────────────────────────────────────
    {
        juce::ColourGradient hdr(
            juce::Colour(0xFF1A1510), 0.0f,  0.0f,
            juce::Colour(0xFF0E0C08), 0.0f, 40.0f, false);
        g.setGradientFill(hdr);
        g.fillRect(0, 0, W, 40);

        g.setColour(juce::Colour(0xFF2E2A1A));
        g.drawHorizontalLine(40, 0.0f, (float)W);
    }

    // Plugin name
    g.setColour(juce::Colour(0xFFFFB300));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 16.0f, juce::Font::bold));
    g.drawText("DUB  SIREN", 20, 0, 260, 40, juce::Justification::centredLeft, false);

    // Sub-brand right side
    g.setColour(juce::Colour(0xFFCFC9A8).withAlpha(0.4f));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain));
    g.drawText("AudioRF  v2.0", W - 130, 0, 120, 40, juce::Justification::centredRight, false);

    // Status LED (green on)
    const float ledX = W - 18.0f;
    const float ledY = 14.0f;
    g.setColour(juce::Colour(0xFF39FF14).withAlpha(0.8f));
    g.fillEllipse(ledX, ledY, 8.0f, 8.0f);
    g.setColour(juce::Colour(0xFF39FF14).withAlpha(0.15f));
    g.fillEllipse(ledX - 3.0f, ledY - 3.0f, 14.0f, 14.0f);

    // ── Section boxes — drawn BEFORE child components ────────────────────
    drawSection(g, { 8,  44, 355, 195 }, "VCO");
    drawSection(g, { 368, 44, 344, 195 }, "DELAY");
    drawSection(g, { 8,   246, 355, 235 }, "FILTER  —  VCF  SWEEP");
    drawSection(g, { 368, 246, 344, 114 }, "LFO  1  —  WOBBLE");
    drawSection(g, { 368, 364, 344, 117 }, "LFO  2  —  SHAKE");

    // ── Corner screws ─────────────────────────────────────────────────────
    const int screwPositions[][2] = { {8,8},{W-14,8},{8,H-14},{W-14,H-14} };
    for (auto& sp : screwPositions)
    {
        g.setColour(juce::Colour(0xFF2A2418));
        g.fillEllipse((float)sp[0], (float)sp[1], 6.0f, 6.0f);
        g.setColour(juce::Colour(0xFF504030));
        g.drawEllipse((float)sp[0], (float)sp[1], 6.0f, 6.0f, 0.8f);
        // Slot
        g.drawLine(sp[0]+3.0f, sp[1]+1.0f, sp[0]+3.0f, sp[1]+5.0f, 0.8f);
    }
}

// ── resized ───────────────────────────────────────────────────────────────────
void DubSirenEditor::resized()
{
    // ── Header buttons ─────────────────────────────────────────────────────
    // MIDI-TARGET | SHOT-select | FIRE | RANDOMIZE
    midiTargetBox  .setBounds(255, 8,  88, 24);
    sampleSelectBox.setBounds(350, 8,  80, 24);
    sampleButton   .setBounds(434, 8,  48, 24);
    randomizeButton.setBounds(490, 8, 100, 24);

    const int knob   = 70;  // knob component size
    const int lh     = 14;  // label height
    const int comboH = 22;
    const int comboW = 100;

    // ──────────────────── VCO section (x=8..363, y=44..239) ────────────────
    // 3 knobs evenly spread, wave combobox centred below — all within border
    const int vcoSectionX = 8;
    const int vcoSectionW = 355;
    // Three equal slots across the section (16px padding each side)
    const int vcoSlotW = (vcoSectionW - 32) / 3;           // ~107
    const int siren_cx = vcoSectionX + 16 + vcoSlotW / 2;  // slot-1 centre
    const int level_cx = siren_cx + vcoSlotW;
    const int porta_cx = level_cx + vcoSlotW;

    const int vcoY    = 65;
    const int vcoBase = vcoY + knob + 2;

    const int siren_x = siren_cx - knob / 2;
    const int level_x = level_cx - knob / 2;
    const int porta_x = porta_cx - knob / 2;

    vcoRateSlider   .setBounds(siren_x, vcoY, knob, knob);
    vcoLevelSlider  .setBounds(level_x, vcoY, knob, knob);
    portamentoSlider.setBounds(porta_x, vcoY, knob, knob);

    vcoRateLabel   .setBounds(siren_x, vcoBase, knob, lh);
    vcoLevelLabel  .setBounds(level_x, vcoBase, knob, lh);
    portamentoLabel.setBounds(porta_x, vcoBase, knob, lh);

    // Wave combo — centred below the three knobs
    const int vcoWaveY  = vcoBase + lh + 8;
    const int vcoWaveX  = vcoSectionX + (vcoSectionW - comboW) / 2;
    vcoWaveLabel   .setBounds(vcoWaveX, vcoWaveY,          comboW, lh);
    vcoWaveformBox .setBounds(vcoWaveX, vcoWaveY + lh + 2, comboW, comboH);

    // ──────────────────── DELAY section (x=368..712, y=44..239) ────────────
    const int dlyY    = 65;
    const int dlyBase = dlyY + knob + 2;

    const int echo_x  = 380;
    const int rep_x   = 468;
    const int mix_x   = 556;

    delayTimeSlider    .setBounds(echo_x, dlyY, knob, knob);
    delayFeedbackSlider.setBounds(rep_x,  dlyY, knob, knob);
    delayWetDrySlider  .setBounds(mix_x,  dlyY, knob, knob);

    delayTimeLabel    .setBounds(echo_x, dlyBase, knob, lh);
    delayFeedbackLabel.setBounds(rep_x,  dlyBase, knob, lh);
    delayWetDryLabel  .setBounds(mix_x,  dlyBase, knob, lh);

    // Sample knobs — lower row of DELAY section (small, 52px)
    const int smpKnob = 52;
    const int smpY    = dlyBase + lh + 8;
    const int smpBase = smpY + smpKnob + 2;
    sampleLevelSlider.setBounds(echo_x, smpY, smpKnob, smpKnob);
    sampleRateSlider .setBounds(rep_x,  smpY, smpKnob, smpKnob);
    sampleLevelLabel .setBounds(echo_x, smpBase, smpKnob + 10, lh);
    sampleRateLabel  .setBounds(rep_x,  smpBase, smpKnob + 10, lh);

    // ──────────────────── FILTER section (x=8..363, y=246..481) ────────────
    // 3 knobs evenly spread — same column as VCO
    const int fltSectionX = 8;
    const int fltSectionW = 355;
    const int fltSlotW    = (fltSectionW - 32) / 3;
    const int sweep_cx    = fltSectionX + 16 + fltSlotW / 2;
    const int reso_cx     = sweep_cx + fltSlotW;
    const int decay_cx    = reso_cx  + fltSlotW;
    const int fltY        = 270;
    const int fltBase     = fltY + knob + 2;

    filterCutoffSlider   .setBounds(sweep_cx - knob / 2, fltY, knob, knob);
    filterResonanceSlider.setBounds(reso_cx  - knob / 2, fltY, knob, knob);
    filterDecaySlider    .setBounds(decay_cx - knob / 2, fltY, knob, knob);

    filterCutoffLabel   .setBounds(sweep_cx - knob / 2, fltBase, knob, lh);
    filterResonanceLabel.setBounds(reso_cx  - knob / 2, fltBase, knob, lh);
    filterDecayLabel    .setBounds(decay_cx - knob / 2, fltBase, knob, lh);

    // ──────────────────── LFO 1 section (x=368..712, y=246..360) ────────────
    // Compact: 2 mini-knobs (60px) + 2 combos stacked to the right
    const int lfoKnob  = 60;
    const int lfoLh    = 12;

    const int lfo1Y    = 258;
    const int lfo1Base = lfo1Y + lfoKnob + 2;
    const int w1_x     = 382;
    const int d1_x     = 454;

    lfo1RateSlider  .setBounds(w1_x, lfo1Y, lfoKnob, lfoKnob);
    lfo1AmountSlider.setBounds(d1_x, lfo1Y, lfoKnob, lfoKnob);
    lfo1RateLabel  .setBounds(w1_x, lfo1Base, lfoKnob, lfoLh);
    lfo1AmountLabel.setBounds(d1_x, lfo1Base, lfoKnob, lfoLh);

    const int lfo1ComboX  = 526;
    const int lfo1Combo1Y = lfo1Y;
    const int lfo1Combo2Y = lfo1Combo1Y + lh + comboH + 6;

    lfo1TargetLabel .setBounds(lfo1ComboX, lfo1Combo1Y,        comboW + 10, lh);
    lfo1TargetBox   .setBounds(lfo1ComboX, lfo1Combo1Y + lh,   comboW + 10, comboH);
    lfo1WaveLabel   .setBounds(lfo1ComboX, lfo1Combo2Y,        comboW + 10, lh);
    lfo1WaveformBox .setBounds(lfo1ComboX, lfo1Combo2Y + lh,   comboW + 10, comboH);

    // ──────────────────── LFO 2 section (x=368..712, y=364..481) ────────────
    const int lfo2Y    = 376;
    const int lfo2Base = lfo2Y + lfoKnob + 2;
    const int w2_x     = 382;
    const int d2_x     = 454;

    lfo2RateSlider  .setBounds(w2_x, lfo2Y, lfoKnob, lfoKnob);
    lfo2AmountSlider.setBounds(d2_x, lfo2Y, lfoKnob, lfoKnob);
    lfo2RateLabel  .setBounds(w2_x, lfo2Base, lfoKnob, lfoLh);
    lfo2AmountLabel.setBounds(d2_x, lfo2Base, lfoKnob, lfoLh);

    const int lfo2ComboX  = 526;
    const int lfo2Combo1Y = lfo2Y;
    const int lfo2Combo2Y = lfo2Combo1Y + lh + comboH + 6;

    lfo2TargetLabel .setBounds(lfo2ComboX, lfo2Combo1Y,        comboW + 10, lh);
    lfo2TargetBox   .setBounds(lfo2ComboX, lfo2Combo1Y + lh,   comboW + 10, comboH);
    lfo2WaveLabel   .setBounds(lfo2ComboX, lfo2Combo2Y,        comboW + 10, lh);
    lfo2WaveformBox .setBounds(lfo2ComboX, lfo2Combo2Y + lh,   comboW + 10, comboH);
}
