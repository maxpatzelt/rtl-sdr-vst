/*
  RTL-SDR Plugin Editor (GUI)
  Simple but functional interface for RF tuning
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class RTLSDRPluginEditor : public juce::AudioProcessorEditor,
                           public juce::Timer
{
public:
    RTLSDRPluginEditor(RTLSDRPluginProcessor&);
    ~RTLSDRPluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    RTLSDRPluginProcessor& audioProcessor;

    // UI Components
    juce::Label titleLabel;
    juce::Label statusLabel;

    juce::Label frequencyLabel;
    juce::Slider frequencySlider;
    juce::TextButton frequencyPreset1, frequencyPreset2, frequencyPreset3;

    juce::Label gainLabel;
    juce::Slider gainSlider;

    juce::Label modeLabel;
    juce::ComboBox modeCombo;

    juce::Label volumeLabel;
    juce::Slider volumeSlider;

    juce::ComboBox deviceCombo;
    juce::TextButton startButton;
    juce::TextButton stopButton;

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;

    void updateDeviceList();
    void updateStatus();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RTLSDRPluginEditor)
};
