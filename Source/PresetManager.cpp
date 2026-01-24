/*
  Preset Manager Implementation
*/

#include "PresetManager.h"

PresetManager::PresetManager()
{
    loadDefaultPresets();
}

PresetManager::~PresetManager()
{
}

void PresetManager::addPreset(const RadioPreset& preset)
{
    presets.push_back(preset);
}

void PresetManager::removePreset(int index)
{
    if (index >= 0 && index < (int)presets.size())
        presets.erase(presets.begin() + index);
}

void PresetManager::updatePreset(int index, const RadioPreset& preset)
{
    if (index >= 0 && index < (int)presets.size())
        presets[index] = preset;
}

const RadioPreset& PresetManager::getPreset(int index) const
{
    static RadioPreset defaultPreset;
    if (index >= 0 && index < (int)presets.size())
        return presets[index];
    return defaultPreset;
}

void PresetManager::saveToFile(const juce::File& file)
{
    juce::XmlElement root("Presets");

    for (const auto& preset : presets)
    {
        auto* presetXml = root.createNewChildElement("Preset");
        presetXml->setAttribute("name", preset.name);
        presetXml->setAttribute("frequency", preset.frequency);
        presetXml->setAttribute("gain", preset.gain);
        presetXml->setAttribute("mode", preset.mode);
    }

    if (!root.writeTo(file))
    {
        juce::Logger::writeToLog("Failed to save presets to: " + file.getFullPathName());
    }
}

void PresetManager::loadFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return;

    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr)
        return;

    presets.clear();

    for (auto* presetXml : xml->getChildIterator())
    {
        if (presetXml->hasTagName("Preset"))
        {
            RadioPreset preset;
            preset.name = presetXml->getStringAttribute("name", "Unnamed");
            preset.frequency = presetXml->getDoubleAttribute("frequency", 100.0);
            preset.gain = (float)presetXml->getDoubleAttribute("gain", 20.0);
            preset.mode = presetXml->getStringAttribute("mode", "FM");

            presets.push_back(preset);
        }
    }
}

void PresetManager::loadDefaultPresets()
{
    presets.clear();

    // FM Radio presets
    presets.push_back(RadioPreset("Classic FM", 100.0, 30.0f, "FM"));
    presets.push_back(RadioPreset("NPR", 88.5, 30.0f, "FM"));
    presets.push_back(RadioPreset("Local Rock", 95.5, 30.0f, "FM"));
    presets.push_back(RadioPreset("Jazz", 90.1, 30.0f, "FM"));

    // Aviation presets
    presets.push_back(RadioPreset("Tower", 118.1, 40.0f, "AM"));
    presets.push_back(RadioPreset("Ground", 121.9, 40.0f, "AM"));
    presets.push_back(RadioPreset("Approach", 119.7, 40.0f, "AM"));
    presets.push_back(RadioPreset("ATIS", 128.05, 40.0f, "AM"));

    // Weather presets
    presets.push_back(RadioPreset("NOAA Weather 1", 162.4, 35.0f, "FM"));
    presets.push_back(RadioPreset("NOAA Weather 2", 162.425, 35.0f, "FM"));
    presets.push_back(RadioPreset("NOAA Weather 3", 162.45, 35.0f, "FM"));
    presets.push_back(RadioPreset("NOAA Weather 4", 162.475, 35.0f, "FM"));

    // Amateur radio presets
    presets.push_back(RadioPreset("2M Calling", 146.52, 35.0f, "FM"));
    presets.push_back(RadioPreset("2M Simplex", 146.4, 35.0f, "FM"));
    presets.push_back(RadioPreset("70cm Calling", 446.0, 35.0f, "FM"));

    // Shortwave presets
    presets.push_back(RadioPreset("WWV 10MHz", 10.0, 40.0f, "AM"));
    presets.push_back(RadioPreset("WWV 15MHz", 15.0, 40.0f, "AM"));
    presets.push_back(RadioPreset("WWV 20MHz", 20.0, 40.0f, "AM"));
}
