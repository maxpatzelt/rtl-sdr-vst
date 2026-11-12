#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// ── Dark dub-studio LookAndFeel ──────────────────────────────────────────────
// Inspired by vintage Jamaican studio rack gear:
//  dark charcoal chassis, amber arc, cream indicator line, bakelite cap.
class DubLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DubLookAndFeel();

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawComboBox(juce::Graphics& g, int width, int height,
                      bool isButtonDown, int buttonX, int buttonY,
                      int buttonW, int buttonH,
                      juce::ComboBox&) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&,
                              bool isMouseOverButton, bool isButtonDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool isMouseOverButton, bool isButtonDown) override;

private:
    // Palette constants
    static constexpr uint32_t CHASSIS   = 0xFF161210;  // near-black warm
    static constexpr uint32_t PANEL     = 0xFF1E1B16;  // section body
    static constexpr uint32_t AMBER     = 0xFFFFB300;  // arc / accent
    static constexpr uint32_t AMBER_DIM = 0xFF3A2800;  // arc track
    static constexpr uint32_t CREAM     = 0xFFCFC9A8;  // indicator / label
    static constexpr uint32_t KNOB_DARK = 0xFF252018;  // knob body
    static constexpr uint32_t KNOB_RIM  = 0xFF383020;  // knob outer ring
    static constexpr uint32_t GREEN_LED = 0xFF39FF14;  // phosphor green accent
};

// ── Editor ───────────────────────────────────────────────────────────────────
class DubSirenEditor : public juce::AudioProcessorEditor
{
public:
    explicit DubSirenEditor(DubSirenProcessor&);
    ~DubSirenEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // ── Helper: draw one engraved section box ───────────────────────────────
    void drawSection(juce::Graphics& g,
                     juce::Rectangle<int> bounds,
                     const juce::String& title) const;

    DubSirenProcessor& processorRef;
    DubLookAndFeel     dubLAF;

    juce::TextButton triggerButton;
    juce::TextButton sampleButton;
    juce::ComboBox   sampleSelectBox;
    juce::ComboBox   midiTargetBox;
    juce::TextButton randomizeButton;
    bool             buttonIsDown_ = false;

    // VCO controls
    juce::Slider   vcoRateSlider, vcoLevelSlider, portamentoSlider;
    juce::ComboBox vcoWaveformBox;
    juce::Label    vcoRateLabel, vcoLevelLabel, portamentoLabel, vcoWaveLabel;

    // Delay controls
    juce::Slider   delayTimeSlider, delayFeedbackSlider, delayWetDrySlider;
    juce::Label    delayTimeLabel, delayFeedbackLabel, delayWetDryLabel;

    // LFO 1 controls
    juce::Slider   lfo1RateSlider, lfo1AmountSlider;
    juce::ComboBox lfo1TargetBox, lfo1WaveformBox;
    juce::Label    lfo1RateLabel, lfo1AmountLabel, lfo1TargetLabel, lfo1WaveLabel;

    // LFO 2 controls
    juce::Slider   lfo2RateSlider, lfo2AmountSlider;
    juce::ComboBox lfo2TargetBox, lfo2WaveformBox;
    juce::Label    lfo2RateLabel, lfo2AmountLabel, lfo2TargetLabel, lfo2WaveLabel;

    // Sample controls
    juce::Slider   sampleLevelSlider, sampleRateSlider;
    juce::Label    sampleLevelLabel,  sampleRateLabel;

    // Filter controls
    juce::Slider   filterCutoffSlider, filterResonanceSlider, filterDecaySlider;
    juce::Label    filterCutoffLabel,  filterResonanceLabel,  filterDecayLabel;

    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ChoiceAttach = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttach> vcoRateAttach, vcoLevelAttach, portamentoAttach;
    std::unique_ptr<SliderAttach> delayTimeAttach, delayFeedbackAttach, delayWetDryAttach;
    std::unique_ptr<SliderAttach> lfo1RateAttach, lfo1AmountAttach;
    std::unique_ptr<SliderAttach> lfo2RateAttach, lfo2AmountAttach;
    std::unique_ptr<SliderAttach> sampleLevelAttach, sampleRateAttach;
    std::unique_ptr<SliderAttach> filterCutoffAttach, filterResonanceAttach, filterDecayAttach;

    std::unique_ptr<ChoiceAttach> vcoWaveAttach;
    std::unique_ptr<ChoiceAttach> lfo1TargetAttach, lfo1WaveAttach;
    std::unique_ptr<ChoiceAttach> lfo2TargetAttach, lfo2WaveAttach;
    std::unique_ptr<ChoiceAttach> midiTargetAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DubSirenEditor)
};
