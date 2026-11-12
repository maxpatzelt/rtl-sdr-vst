/*
  Frequency History - Recent frequency memory
*/

#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <deque>

struct FrequencyHistoryEntry
{
    double frequency;  // MHz
    juce::Time timestamp;
    juce::String label;  // Optional user label

    FrequencyHistoryEntry(double freq, const juce::String& lbl = "")
        : frequency(freq), timestamp(juce::Time::getCurrentTime()), label(lbl) {}
};

class FrequencyHistory
{
public:
    FrequencyHistory(int maxEntries = 20);
    ~FrequencyHistory();

    // Add a frequency to history
    void addFrequency(double frequency, const juce::String& label = "");

    // Get recent frequencies
    const std::deque<FrequencyHistoryEntry>& getHistory() const { return history; }

    // Get frequency at index
    const FrequencyHistoryEntry* getEntry(int index) const;

    // Clear history
    void clear();

    // Get number of entries
    int size() const { return (int)history.size(); }

    // Check if frequency is already in recent history (within 0.1 MHz)
    bool hasRecent(double frequency, double toleranceMHz = 0.1) const;

    // Save/load from file
    void saveToFile(const juce::File& file);
    void loadFromFile(const juce::File& file);

private:
    std::deque<FrequencyHistoryEntry> history;
    int maxEntries;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyHistory)
};
