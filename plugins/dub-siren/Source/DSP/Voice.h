#pragma once

#include "Oscillator.h"
#include "Envelope.h"

namespace DubSiren {
namespace DSP {

/**
 * Synthesizer Voice
 *
 * Combines an oscillator and envelope to create a complete monophonic voice.
 * Handles MIDI note on/off events and renders audio output.
 *
 * Architecture follows Mutable Instruments paradigm:
 * - Voice owns its DSP components (oscillator, envelope)
 * - Clean separation between parameter updates and audio rendering
 * - Sample-accurate gate timing
 *
 * Future extensions:
 * - Add filter
 * - Add modulation sources (LFO, etc.)
 * - Polyphony management
 */
class Voice {
public:
    Voice();
    ~Voice() = default;

    /**
     * Initialize voice with sample rate.
     */
    void Init(float sampleRate);

    /**
     * Trigger note on.
     * midiNote: MIDI note number (0-127)
     * velocity: Note velocity (0.0 to 1.0)
     */
    void NoteOn(int midiNote, float velocity);

    /**
     * Trigger note off.
     */
    void NoteOff();

    /**
     * Reset voice to initial state.
     */
    void Reset();

    /**
     * Process a block of audio samples.
     * output: Buffer to write audio to
     * numSamples: Number of samples to generate
     */
    void Process(float* output, size_t numSamples);

    /**
     * Check if voice is active (envelope not idle).
     */
    bool IsActive() const;

    /**
     * Set oscillator waveform.
     */
    void SetOscillatorWaveform(Oscillator::Waveform waveform);

    /**
     * Set envelope parameters.
     */
    void SetEnvelopeParameters(float attackMs, float decayMs,
                               float sustainLevel, float releaseMs);

    // Getters for testing
    int GetCurrentNote() const { return currentNote_; }
    float GetVelocity() const { return velocity_; }
    const Oscillator& GetOscillator() const { return oscillator_; }
    const Envelope& GetEnvelope() const { return envelope_; }

private:
    Oscillator oscillator_;
    Envelope envelope_;

    int currentNote_;    // Current MIDI note number
    float velocity_;     // Current velocity (0.0 to 1.0)
    float sampleRate_;
};

} // namespace DSP
} // namespace DubSiren
