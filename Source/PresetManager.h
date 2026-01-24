/*
  Preset Manager - Save and load favorite stations
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

struct RadioPreset
{
    juce::String name;
    double frequency;  // MHz
    float gain;
    juce::String mode;  // "FM", "AM", "USB", "LSB"

    RadioPreset() : frequency(100.0), gain(20.0f), mode("FM") {}

    RadioPreset(const juce::String& n, double f, float g, const juce::String& m)
        : name(n), frequency(f), gain(g), mode(m) {}
};

class PresetManager
{
public:
    PresetManager();
    ~PresetManager();

    void addPreset(const RadioPreset& preset);
    void removePreset(int index);
    void updatePreset(int index, const RadioPreset& preset);

    const RadioPreset& getPreset(int index) const;
    int getNumPresets() const { return (int)presets.size(); }

    void saveToFile(const juce::File& file);
    void loadFromFile(const juce::File& file);

    void loadDefaultPresets();

    std::vector<RadioPreset>& getPresets() { return presets; }
    const std::vector<RadioPreset>& getPresets() const { return presets; }

private:
    std::vector<RadioPreset> presets;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
