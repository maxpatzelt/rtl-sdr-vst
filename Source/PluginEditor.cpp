/*
  RTL-SDR Plugin Editor Implementation
  Modern UI with spectrum analyzer, waterfall, and preset management
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

RTLSDRPluginEditor::RTLSDRPluginEditor(RTLSDRPluginProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(800, 700);

    // Add visualization components
    addAndMakeVisible(spectrumAnalyzer);
    addAndMakeVisible(waterfallDisplay);
    addAndMakeVisible(signalMeter);

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
        {
            audioProcessor.frequencyParam->setValueNotifyingHost(frequencySlider.getValue() / 1766.0f);
            spectrumAnalyzer.setCenterFrequency(frequencySlider.getValue() * 1e6);
            waterfallDisplay.setCenterFrequency(frequencySlider.getValue() * 1e6);
        }
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
    modeCombo.addItem("USB", 4);
    modeCombo.addItem("LSB", 5);
    modeCombo.setSelectedId(1);
    modeCombo.onChange = [this]() {
        if (audioProcessor.modeParam)
            audioProcessor.modeParam->setValueNotifyingHost((float)(modeCombo.getSelectedId() - 1) / 4.0f);
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

    // Preset UI
    presetCombo.setTextWhenNothingSelected("Select Preset");
    presetCombo.onChange = [this]() {
        int selectedId = presetCombo.getSelectedId();
        if (selectedId > 0)
            loadPreset(selectedId - 1);
    };
    addAndMakeVisible(presetCombo);

    savePresetButton.setButtonText("Save Preset");
    savePresetButton.onClick = [this]() { saveCurrentAsPreset(); };
    addAndMakeVisible(savePresetButton);

    deletePresetButton.setButtonText("Delete");
    deletePresetButton.onClick = [this]() { deleteCurrentPreset(); };
    addAndMakeVisible(deletePresetButton);

    // History combo box
    historyCombo.setTextWhenNothingSelected("Recent Frequencies");
    historyCombo.onChange = [this]() {
        int selectedId = historyCombo.getSelectedId();
        if (selectedId > 0)
        {
            auto* entry = frequencyHistory.getEntry(selectedId - 1);
            if (entry)
                frequencySlider.setValue(entry->frequency);
        }
    };
    addAndMakeVisible(historyCombo);

    // Setup click-to-tune callback
    spectrumAnalyzer.onFrequencyClicked = [this](double freq) {
        frequencySlider.setValue(freq);
        addToHistory(freq);
    };

    // Update device and preset lists
    updateDeviceList();
    updatePresetList();

    // Start timer for status updates
    startTimer(100);
}

RTLSDRPluginEditor::~RTLSDRPluginEditor()
{
    stopTimer();
}

void RTLSDRPluginEditor::paint(juce::Graphics& g)
{
    // Modern dark theme
    g.fillAll(juce::Colour(0xFF1a1a1a));

    // Background panels
    g.setColour(juce::Colour(0xFF2a2a2a));
    g.fillRoundedRectangle(10, 50, getWidth() - 20, 280, 5.0f);  // Visualization panel
    g.fillRoundedRectangle(10, 340, getWidth() - 20, 350, 5.0f);  // Controls panel
}

void RTLSDRPluginEditor::resized()
{
    auto bounds = getLocalBounds().reduced(15);

    // Title
    titleLabel.setBounds(bounds.removeFromTop(35));

    bounds.removeFromTop(10);

    // Spectrum analyzer (top third)
    spectrumAnalyzer.setBounds(bounds.removeFromTop(120).reduced(5));

    bounds.removeFromTop(5);

    // Waterfall display (middle third)
    waterfallDisplay.setBounds(bounds.removeFromTop(120).reduced(5));

    bounds.removeFromTop(10);

    // Signal meter
    signalMeter.setBounds(bounds.removeFromTop(30).reduced(5));

    bounds.removeFromTop(15);

    // Status
    statusLabel.setBounds(bounds.removeFromTop(25));

    bounds.removeFromTop(10);

    // Device selection row
    auto deviceRow = bounds.removeFromTop(30);
    deviceCombo.setBounds(deviceRow.removeFromLeft(300));
    deviceRow.removeFromLeft(10);
    startButton.setBounds(deviceRow.removeFromLeft(100));
    deviceRow.removeFromLeft(10);
    stopButton.setBounds(deviceRow.removeFromLeft(100));

    bounds.removeFromTop(15);

    // Preset row
    auto presetRow = bounds.removeFromTop(30);
    juce::Label* presetLabel = new juce::Label();
    presetLabel->setText("Presets:", juce::dontSendNotification);
    presetLabel->setBounds(presetRow.removeFromLeft(80));
    addAndMakeVisible(presetLabel);
    presetCombo.setBounds(presetRow.removeFromLeft(250));
    presetRow.removeFromLeft(10);
    savePresetButton.setBounds(presetRow.removeFromLeft(100));
    presetRow.removeFromLeft(10);
    deletePresetButton.setBounds(presetRow.removeFromLeft(80));

    bounds.removeFromTop(15);

    // Frequency
    frequencyLabel.setBounds(bounds.removeFromTop(20));
    frequencySlider.setBounds(bounds.removeFromTop(30));

    auto presetBtnRow = bounds.removeFromTop(30);
    frequencyPreset1.setBounds(presetBtnRow.removeFromLeft(180));
    presetBtnRow.removeFromLeft(5);
    frequencyPreset2.setBounds(presetBtnRow.removeFromLeft(180));
    presetBtnRow.removeFromLeft(5);
    frequencyPreset3.setBounds(presetBtnRow.removeFromLeft(180));

    bounds.removeFromTop(15);

    // Gain
    gainLabel.setBounds(bounds.removeFromTop(20));
    gainSlider.setBounds(bounds.removeFromTop(30));

    bounds.removeFromTop(15);

    // Mode
    auto modeRow = bounds.removeFromTop(30);
    modeLabel.setBounds(modeRow.removeFromLeft(100));
    modeCombo.setBounds(modeRow.removeFromLeft(200));

    bounds.removeFromTop(15);

    // Volume
    volumeLabel.setBounds(bounds.removeFromTop(20));
    volumeSlider.setBounds(bounds.removeFromTop(30));
}

void RTLSDRPluginEditor::timerCallback()
{
    updateStatus();

    // Feed data to visualizers if SDR is running
    if (audioProcessor.isSDRRunning())
    {
        // Get IQ samples from the processor's ring buffer
        auto& iqBuffer = audioProcessor.getIQBuffer();
        const int samplesToRead = 2048;
        uint8_t buffer[samplesToRead];

        int samplesRead = iqBuffer.read(buffer, samplesToRead);
        if (samplesRead > 0)
        {
            spectrumAnalyzer.pushSamples(buffer, samplesRead);
            waterfallDisplay.pushSamples(buffer, samplesRead);
            signalMeter.updateSignal(buffer, samplesRead);
        }
    }
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
        if (audioProcessor.frequencyParam)
        {
            float freq = audioProcessor.frequencyParam->get();
            frequencySlider.setValue(freq, juce::dontSendNotification);
        }

        // Update gain
        if (audioProcessor.gainParam)
            gainSlider.setValue(audioProcessor.gainParam->get(), juce::dontSendNotification);

        // Update volume
        if (audioProcessor.volumeParam)
            volumeSlider.setValue(audioProcessor.volumeParam->get(), juce::dontSendNotification);
    }
}

void RTLSDRPluginEditor::updatePresetList()
{
    presetCombo.clear();

    const auto& presets = presetManager.getPresets();
    for (int i = 0; i < presetManager.getNumPresets(); ++i)
    {
        presetCombo.addItem(presets[i].name, i + 1);
    }
}

void RTLSDRPluginEditor::loadPreset(int index)
{
    if (index < 0 || index >= presetManager.getNumPresets())
        return;

    const auto& preset = presetManager.getPreset(index);

    // Apply preset settings
    frequencySlider.setValue(preset.frequency);
    gainSlider.setValue(preset.gain);

    // Set mode
    if (preset.mode == "FM")
        modeCombo.setSelectedId(1);
    else if (preset.mode == "AM")
        modeCombo.setSelectedId(3);
}

void RTLSDRPluginEditor::saveCurrentAsPreset()
{
    // Use async alert window instead of deprecated runModalLoop
    juce::AlertWindow::showAsync(
        juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::QuestionIcon)
            .withTitle("Save Preset")
            .withMessage("Enter preset name:")
            .withButton("OK")
            .withButton("Cancel"),
        [this](int result)
        {
            if (result == 1)  // OK button
            {
                // For simplicity, use a default name for now
                // In a real app, you'd use a proper input dialog
                RadioPreset preset;
                preset.name = "Preset " + juce::String(presetManager.getNumPresets() + 1);
                preset.frequency = frequencySlider.getValue();
                preset.gain = (float)gainSlider.getValue();

                int modeId = modeCombo.getSelectedId();
                preset.mode = (modeId == 3) ? "AM" : "FM";

                presetManager.addPreset(preset);
                updatePresetList();
            }
        }
    );
}

void RTLSDRPluginEditor::deleteCurrentPreset()
{
    int selectedId = presetCombo.getSelectedId();
    if (selectedId > 0)
    {
        presetManager.removePreset(selectedId - 1);
        updatePresetList();
        presetCombo.setSelectedId(0);
    }
}

void RTLSDRPluginEditor::updateHistoryList()
{
    historyCombo.clear();

    const auto& history = frequencyHistory.getHistory();
    int id = 1;
    for (const auto& entry : history)
    {
        juce::String text = juce::String(entry.frequency, 3) + " MHz";
        if (entry.label.isNotEmpty())
            text += " - " + entry.label;
        historyCombo.addItem(text, id++);
    }
}

void RTLSDRPluginEditor::addToHistory(double frequency)
{
    frequencyHistory.addFrequency(frequency);
    updateHistoryList();
}
