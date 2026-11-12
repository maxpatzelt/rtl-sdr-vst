/*
  Frequency History Implementation
*/

#include "FrequencyHistory.h"

FrequencyHistory::FrequencyHistory(int max)
    : maxEntries(max)
{
}

FrequencyHistory::~FrequencyHistory()
{
}

void FrequencyHistory::addFrequency(double frequency, const juce::String& label)
{
    // Don't add if it's already recent (within 0.1 MHz)
    if (hasRecent(frequency, 0.1))
        return;

    // Add to front of history
    history.push_front(FrequencyHistoryEntry(frequency, label));

    // Remove oldest if exceeds max
    while (history.size() > (size_t)maxEntries)
        history.pop_back();
}

const FrequencyHistoryEntry* FrequencyHistory::getEntry(int index) const
{
    if (index >= 0 && index < (int)history.size())
        return &history[index];
    return nullptr;
}

void FrequencyHistory::clear()
{
    history.clear();
}

bool FrequencyHistory::hasRecent(double frequency, double toleranceMHz) const
{
    for (const auto& entry : history)
    {
        if (std::abs(entry.frequency - frequency) < toleranceMHz)
            return true;
    }
    return false;
}

void FrequencyHistory::saveToFile(const juce::File& file)
{
    juce::XmlElement root("FrequencyHistory");

    for (const auto& entry : history)
    {
        auto* entryXml = root.createNewChildElement("Entry");
        entryXml->setAttribute("frequency", entry.frequency);
        entryXml->setAttribute("timestamp", (int)entry.timestamp.toMilliseconds());
        if (entry.label.isNotEmpty())
            entryXml->setAttribute("label", entry.label);
    }

    if (!root.writeTo(file))
    {
        juce::Logger::writeToLog("Failed to save frequency history to: " + file.getFullPathName());
    }
}

void FrequencyHistory::loadFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return;

    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr)
        return;

    history.clear();

    for (auto* entryXml : xml->getChildIterator())
    {
        if (entryXml->hasTagName("Entry"))
        {
            double freq = entryXml->getDoubleAttribute("frequency", 100.0);
            juce::String label = entryXml->getStringAttribute("label", "");

            FrequencyHistoryEntry entry(freq, label);
            entry.timestamp = juce::Time((juce::int64)entryXml->getIntAttribute("timestamp", 0));

            history.push_back(entry);
        }
    }

    // Trim to max entries
    while (history.size() > (size_t)maxEntries)
        history.pop_back();
}
