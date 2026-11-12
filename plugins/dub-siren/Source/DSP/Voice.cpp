#include "Voice.h"
#include <cassert>

namespace DubSiren {
namespace DSP {

Voice::Voice()
    : currentNote_(-1)
    , velocity_(0.0f)
    , sampleRate_(44100.0f)
{
}

void Voice::Init(float sampleRate) {
    assert(sampleRate > 0.0f && "Sample rate must be positive");
    sampleRate_ = sampleRate;

    oscillator_.Init(sampleRate);
    envelope_.Init(sampleRate);

    Reset();
}

void Voice::NoteOn(int midiNote, float velocity) {
    assert(midiNote >= 0 && midiNote <= 127 && "MIDI note must be in range 0-127");

    currentNote_ = midiNote;
    velocity_ = Clamp(velocity, 0.0f, 1.0f);

    // Convert MIDI note to frequency and set oscillator
    float frequency = MidiNoteToFrequency(midiNote);
    oscillator_.SetFrequency(frequency);

    // Trigger envelope
    envelope_.NoteOn();
}

void Voice::NoteOff() {
    envelope_.NoteOff();
}

void Voice::Reset() {
    oscillator_.Reset();
    envelope_.Reset();
    currentNote_ = -1;
    velocity_ = 0.0f;
}

void Voice::Process(float* output, size_t numSamples) {
    assert(output != nullptr && "Output buffer cannot be null");

    // If envelope is idle, output silence
    if (!envelope_.IsActive()) {
        for (size_t i = 0; i < numSamples; ++i) {
            output[i] = 0.0f;
        }
        return;
    }

    // Generate oscillator output
    oscillator_.Process(output, numSamples);

    // Apply envelope and velocity
    for (size_t i = 0; i < numSamples; ++i) {
        float envelopeLevel = envelope_.ProcessSample();
        output[i] *= envelopeLevel * velocity_;
    }
}

bool Voice::IsActive() const {
    return envelope_.IsActive();
}

void Voice::SetOscillatorWaveform(Oscillator::Waveform waveform) {
    oscillator_.SetWaveform(waveform);
}

void Voice::SetEnvelopeParameters(float attackMs, float decayMs,
                                  float sustainLevel, float releaseMs) {
    envelope_.SetParameters(attackMs, decayMs, sustainLevel, releaseMs);
}

} // namespace DSP
} // namespace DubSiren
