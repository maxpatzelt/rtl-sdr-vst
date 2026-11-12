#pragma once

#include <cmath>
#include <algorithm>
#include <limits>

namespace DubSiren {
namespace DSP {

// Audio processing constants
constexpr float kMinFrequency = 20.0f;
constexpr float kMaxFrequency = 20000.0f;
constexpr size_t kMaxBlockSize = 512;

// Mathematical constants
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 2.0f * kPi;

// Denormal prevention threshold
constexpr float kDenormalThreshold = 1e-15f;

// MIDI note number for A4 = 440 Hz
constexpr int kMidiA4 = 69;
constexpr float kA4Frequency = 440.0f;

/**
 * Convert MIDI note number to frequency in Hz.
 * Uses equal temperament tuning: f = 440 * 2^((n-69)/12)
 */
inline float MidiNoteToFrequency(int midiNote) {
    return kA4Frequency * std::pow(2.0f, (midiNote - kMidiA4) / 12.0f);
}

/**
 * Clamp a value between minimum and maximum.
 */
template<typename T>
inline T Clamp(T value, T min, T max) {
    return std::max(min, std::min(max, value));
}

/**
 * Prevent denormal floating point values.
 * Returns 0 if absolute value is below threshold.
 */
inline float PreventDenormal(float value) {
    if (std::abs(value) < kDenormalThreshold) {
        return 0.0f;
    }
    return value;
}

/**
 * Linear interpolation between two values.
 */
inline float Lerp(float a, float b, float t) {
    return a + t * (b - a);
}

/**
 * Wrap a phase value to the range [0, 1).
 */
inline float WrapPhase(float phase) {
    return phase - std::floor(phase);
}

} // namespace DSP
} // namespace DubSiren
