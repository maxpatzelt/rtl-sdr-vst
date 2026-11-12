/*
  RTL-SDR Plugin Editor v3.0 — Retro Ham Radio Aesthetic
  Inspired by: Kenwood TS-830S, Heathkit SB-102, Collins KWM-2A

  Colour palette:
    CHASSIS_BG  #0D0D09  — near-black olive, like anodised aluminium under dim light
    PANEL_FACE  #18180F  — dark olive panel face
    DISPLAY_BG  #030604  — near-black display background (phosphor tube)
    AMBER       #FFB300  — warm amber — VFO digits, active indicators
    AMBER_DIM   #1A0D00  — dead amber segment (very dim)
    PHOSPHOR    #39FF14  — phosphor green — spectrum curve peak
    PHOSPHOR_DIM #023302 — dim phosphor (grid / inactive)
    CREAM       #CFC9A8  — cream panel labels
    KNOB_DARK   #1E1B12  — bakelite knob body
    STEEL       #3A3828  — steel hardware accents
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "SpectrumAnalyzer.h"
#include "WaterfallDisplay.h"
#include "SignalMeter.h"
#include "PresetManager.h"
#include "FrequencyHistory.h"

//==============================================================================
class RetroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Palette
    static const juce::Colour CHASSIS_BG;
    static const juce::Colour PANEL_FACE;
    static const juce::Colour DISPLAY_BG;
    static const juce::Colour AMBER;
    static const juce::Colour AMBER_DIM;
    static const juce::Colour PHOSPHOR;
    static const juce::Colour CREAM;
    static const juce::Colour KNOB_DARK;
    static const juce::Colour STEEL;

    RetroLookAndFeel();

    // Rotary knobs — bakelite style with amber arc
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;

    // Buttons — illuminated legend style
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                                bool isMouseOver, bool isButtonDown) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool isMouseOver,
                         bool isButtonDown) override;

    // ComboBox — steel panel style
    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;
    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;

    // Labels — cream on dark
    void drawLabel (juce::Graphics&, juce::Label&) override;
    juce::Font getLabelFont (juce::Label&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RetroLookAndFeel)
};

//==============================================================================
class RTLSDRPluginEditor : public juce::AudioProcessorEditor,
                           public juce::Timer
{
public:
    RTLSDRPluginEditor (RTLSDRPluginProcessor&);
    ~RTLSDRPluginEditor() override;

    void paint      (juce::Graphics&) override;
    void resized    () override;
    void timerCallback() override;

private:
    RTLSDRPluginProcessor& audioProcessor;
    RetroLookAndFeel       retroLNF;

    //==========================================================================
    // Visualization
    SpectrumAnalyzer spectrumAnalyzer;
    WaterfallDisplay waterfallDisplay;
    SignalMeter      signalMeter;

    // Preset / history state
    PresetManager    presetManager;
    FrequencyHistory frequencyHistory;

    //==========================================================================
    // ── Controls ──────────────────────────────────────────────────────────────

    // Gain knob
    juce::Slider  gainKnob;
    juce::Label   gainLabel;

    // Volume knob
    juce::Slider  volumeKnob;
    juce::Label   volumeLabel;

    // Squelch knob
    juce::Slider  squelchKnob;
    juce::Label   squelchLabel;

    // Fine-tune knob (±25 kHz)
    juce::Slider  fineTuneKnob;
    juce::Label   fineTuneLabel;

    // Tape saturation knob
    juce::Slider  saturationKnob;
    juce::Label   saturationLabel;

    // AGC toggle button
    juce::TextButton agcButton { "AGC" };

    //==========================================================================
    // ── Mode selection buttons ─────────────────────────────────────────────────
    juce::TextButton modeWFM { "WFM" };
    juce::TextButton modeNFM { "NFM" };
    juce::TextButton modeAM  { "AM"  };
    juce::TextButton modeUSB { "USB" };
    juce::TextButton modeLSB { "LSB" };

    // ── Band preset buttons ────────────────────────────────────────────────────
    juce::TextButton bandFM  { "FM"  };
    juce::TextButton bandAir { "AIR" };
    juce::TextButton bandWx  { "WX"  };
    juce::TextButton bandHam { "HAM" };
    juce::TextButton bandHF  { "HF"  };

    //==========================================================================
    // ── Transport ─────────────────────────────────────────────────────────────
    juce::ComboBox   deviceCombo;
    juce::TextButton startButton { "▶  ON AIR" };
    juce::TextButton stopButton  { "■  STOP"   };
    juce::Label      statusLabel;

    //==========================================================================
    // ── Presets / history ─────────────────────────────────────────────────────
    juce::ComboBox   presetCombo;
    juce::TextButton savePresetButton   { "SAVE"   };
    juce::TextButton deletePresetButton { "DELETE" };
    juce::ComboBox   historyCombo;

    //==========================================================================
    // Internal state
    int    currentModeIndex = 0;
    double displayedFreqMHz = 100.0;

    //==========================================================================
    // Drawing helpers
    void drawHeader        (juce::Graphics&);
    void drawVFODisplay    (juce::Graphics&, juce::Rectangle<int> bounds);
    void drawSevenSegDigit (juce::Graphics&, float x, float y, float w, float h, char c);
    void drawEngravedLine  (juce::Graphics&, int x1, int y, int x2);
    void drawSectionLabel  (juce::Graphics&, juce::Rectangle<int> r, const juce::String& text);
    void drawScrew         (juce::Graphics&, float cx, float cy);
    void styleKnob         (juce::Slider&, const juce::String& name, double lo, double hi,
                            double def, const juce::String& suffix = "");

    // UI logic
    void setMode         (int modeIndex);
    void updateModeButtons();
    void tuneToFrequency (double mhz);
    void updateDeviceList();
    void updatePresetList();
    void updateHistoryList();
    void loadPreset      (int index);
    void saveCurrentAsPreset();
    void deleteCurrentPreset();
    void addToHistory    (double frequencyMHz);
    void updateStatus();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RTLSDRPluginEditor)
};
