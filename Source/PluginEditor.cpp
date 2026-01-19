/*
  RTL-SDR Plugin Editor Implementation
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

RTLSDRPluginEditor::RTLSDRPluginEditor(RTLSDRPluginProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(600, 500);

    // Title
    titleLabel.setText("RTL-SDR Radio", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Status
    statusLabel.setText("Stopped", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    // Frequency controls
    frequencyLabel.setText("Frequency (MHz):", juce::dontSendNotification);
    addAndMakeVisible(frequencyLabel);

    frequencySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    frequencySlider.setRange(24.0, 1766.0, 0.001);
    frequencySlider.setValue(audioProcessor.frequencyParam ? audioProcessor.frequencyParam->get() : 100.0);
    frequencySlider.setNumDecimalPlacesToDisplay(3);
    frequencySlider.onValueChange = [this]() {
        if (audioProcessor.frequencyParam)
            audioProcessor.frequencyParam->setValueNotifyingHost(frequencySlider.getValue() / 1766.0f);
    };
    addAndMakeVisible(frequencySlider);

    // Frequency presets
    frequencyPreset1.setButtonText("FM: 98.7 MHz");
    frequencyPreset1.onClick = [this]() { frequencySlider.setValue(98.7); };
    addAndMakeVisible(frequencyPreset1);

    frequencyPreset2.setButtonText("Aviation: 121.5 MHz");
    frequencyPreset2.onClick = [this]() { frequencySlider.setValue(121.5); };
    addAndMakeVisible(frequencyPreset2);

    frequencyPreset3.setButtonText("Weather: 162.5 MHz");
    frequencyPreset3.onClick = [this]() { frequencySlider.setValue(162.5); };
    addAndMakeVisible(frequencyPreset3);

    // Gain
    gainLabel.setText("Gain (dB):", juce::dontSendNotification);
    addAndMakeVisible(gainLabel);

    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    gainSlider.setRange(0.0, 50.0, 0.1);
    gainSlider.setValue(audioProcessor.gainParam ? audioProcessor.gainParam->get() : 29.0);
    gainSlider.onValueChange = [this]() {
        if (audioProcessor.gainParam)
            audioProcessor.gainParam->setValueNotifyingHost(gainSlider.getValue() / 50.0f);
    };
    addAndMakeVisible(gainSlider);

    // Mode
    modeLabel.setText("Mode:", juce::dontSendNotification);
    addAndMakeVisible(modeLabel);

    modeCombo.addItem("FM (Wideband)", 1);
    modeCombo.addItem("FM (Narrowband)", 2);
    modeCombo.addItem("AM", 3);
    modeCombo.setSelectedId(1);
    modeCombo.onChange = [this]() {
        if (audioProcessor.modeParam)
            audioProcessor.modeParam->setValueNotifyingHost((float)(modeCombo.getSelectedId() - 1) / 2.0f);
    };
    addAndMakeVisible(modeCombo);

    // Volume
    volumeLabel.setText("Volume:", juce::dontSendNotification);
    addAndMakeVisible(volumeLabel);

    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(audioProcessor.volumeParam ? audioProcessor.volumeParam->get() : 0.7);
    volumeSlider.onValueChange = [this]() {
        if (audioProcessor.volumeParam)
            audioProcessor.volumeParam->setValueNotifyingHost(volumeSlider.getValue());
    };
    addAndMakeVisible(volumeSlider);

    // Device selection
    deviceCombo.addItem("No devices", 1);
    addAndMakeVisible(deviceCombo);

    startButton.setButtonText("Start SDR");
    startButton.onClick = [this]() {
        int deviceIndex = deviceCombo.getSelectedId() - 2;
        if (deviceIndex >= 0)
        {
            if (audioProcessor.startSDR(deviceIndex))
            {
                statusLabel.setText("Running: " + deviceCombo.getText(), juce::dontSendNotification);
                statusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
            }
            else
            {
                statusLabel.setText("Failed to start", juce::dontSendNotification);
                statusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
            }
        }
    };
    addAndMakeVisible(startButton);

    stopButton.setButtonText("Stop SDR");
    stopButton.onClick = [this]() {
        audioProcessor.stopSDR();
        statusLabel.setText("Stopped", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    };
    addAndMakeVisible(stopButton);

    // Update device list
    updateDeviceList();

    // Start timer for status updates
    startTimer(100);
}

RTLSDRPluginEditor::~RTLSDRPluginEditor()
{
    stopTimer();
}

void RTLSDRPluginEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // Draw RF spectrum visualization placeholder
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(20, 360, 560, 120);

    g.setColour(juce::Colours::lightblue);
    g.drawText("RF Spectrum (Placeholder)", 20, 360, 560, 30, juce::Justification::centred);
}

void RTLSDRPluginEditor::resized()
{
    auto bounds = getLocalBounds().reduced(20);

    // Title and status
    titleLabel.setBounds(bounds.removeFromTop(40));
    statusLabel.setBounds(bounds.removeFromTop(30));

    bounds.removeFromTop(10);

    // Device selection
    auto deviceRow = bounds.removeFromTop(30);
    deviceCombo.setBounds(deviceRow.removeFromLeft(300));
    deviceRow.removeFromLeft(10);
    startButton.setBounds(deviceRow.removeFromLeft(100));
    deviceRow.removeFromLeft(10);
    stopButton.setBounds(deviceRow.removeFromLeft(100));

    bounds.removeFromTop(20);

    // Frequency
    auto freqLabelRow = bounds.removeFromTop(25);
    frequencyLabel.setBounds(freqLabelRow);

    auto freqRow = bounds.removeFromTop(30);
    frequencySlider.setBounds(freqRow);

    auto presetRow = bounds.removeFromTop(30);
    frequencyPreset1.setBounds(presetRow.removeFromLeft(180));
    presetRow.removeFromLeft(5);
    frequencyPreset2.setBounds(presetRow.removeFromLeft(180));
    presetRow.removeFromLeft(5);
    frequencyPreset3.setBounds(presetRow.removeFromLeft(180));

    bounds.removeFromTop(15);

    // Gain
    gainLabel.setBounds(bounds.removeFromTop(25));
    gainSlider.setBounds(bounds.removeFromTop(30));

    bounds.removeFromTop(15);

    // Mode
    auto modeRow = bounds.removeFromTop(30);
    modeLabel.setBounds(modeRow.removeFromLeft(100));
    modeCombo.setBounds(modeRow.removeFromLeft(200));

    bounds.removeFromTop(15);

    // Volume
    volumeLabel.setBounds(bounds.removeFromTop(25));
    volumeSlider.setBounds(bounds.removeFromTop(30));
}

void RTLSDRPluginEditor::timerCallback()
{
    updateStatus();
}

void RTLSDRPluginEditor::updateDeviceList()
{
    deviceCombo.clear();

    auto devices = audioProcessor.getAvailableDevices();

    if (devices.empty())
    {
        deviceCombo.addItem("No RTL-SDR devices found", 1);
        deviceCombo.setSelectedId(1);
    }
    else
    {
        for (size_t i = 0; i < devices.size(); i++)
        {
            deviceCombo.addItem(devices[i].name + " (" + devices[i].serial + ")", (int)i + 2);
        }
        deviceCombo.setSelectedId(2);
    }
}

void RTLSDRPluginEditor::updateStatus()
{
    if (audioProcessor.isSDRRunning())
    {
        // Update frequency display
        float freq = audioProcessor.frequencyParam->get();
        frequencySlider.setValue(freq, juce::dontSendNotification);

        // Update gain
        gainSlider.setValue(audioProcessor.gainParam->get(), juce::dontSendNotification);

        // Update volume
        volumeSlider.setValue(audioProcessor.volumeParam->get(), juce::dontSendNotification);
    }
}
