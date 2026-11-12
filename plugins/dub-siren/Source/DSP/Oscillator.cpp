#include "Oscillator.h"
#include <cassert>

namespace DubSiren {
namespace DSP {

Oscillator::Oscillator()
    : sampleRate_(44100.0f)
    , frequency_(440.0f)
    , phase_(0.0f)
    , phaseIncrement_(0.0f)
    , waveform_(Waveform::Sine)
{
}

void Oscillator::Init(float sampleRate) {
    assert(sampleRate > 0.0f && "Sample rate must be positive");
    sampleRate_ = sampleRate;

    // Recalculate phase increment with new sample rate
    phaseIncrement_ = frequency_ / sampleRate_;
    phase_ = 0.0f;
}

void Oscillator::SetFrequency(float frequency) {
    // Clamp to valid audio range and below Nyquist
    frequency_ = Clamp(frequency, kMinFrequency,
                       std::min(kMaxFrequency, sampleRate_ * 0.5f));
    phaseIncrement_ = frequency_ / sampleRate_;
}

void Oscillator::SetWaveform(Waveform waveform) {
    waveform_ = waveform;
}

void Oscillator::Reset() {
    phase_ = 0.0f;
}

float Oscillator::PolyBLEP(float t) const {
    // PolyBLEP correction for discontinuities
    // Applied when t is near 0 (discontinuity point)

    if (t < phaseIncrement_) {
        // Discontinuity is happening now
        t /= phaseIncrement_;
        return t + t - t * t - 1.0f;
    } else if (t > 1.0f - phaseIncrement_) {
        // Discontinuity will happen next sample
        t = (t - 1.0f) / phaseIncrement_;
        return t * t + t + t + 1.0f;
    }

    return 0.0f;
}

float Oscillator::GenerateSample() {
    float sample = 0.0f;

    switch (waveform_) {
        case Waveform::Sine: {
            // Pure sine wave - no aliasing, no polyBLEP needed
            sample = std::sin(kTwoPi * phase_);
            break;
        }

        case Waveform::Saw: {
            // Naive sawtooth: ramp from -1 to +1
            sample = 2.0f * phase_ - 1.0f;

            // Apply polyBLEP to smooth the discontinuity at phase wraparound
            sample -= PolyBLEP(phase_);
            break;
        }

        case Waveform::Square: {
            // Naive square wave
            sample = (phase_ < 0.5f) ? 1.0f : -1.0f;

            // Apply polyBLEP at both edges (0.0 and 0.5)
            sample += PolyBLEP(phase_);
            sample -= PolyBLEP(WrapPhase(phase_ + 0.5f));
            break;
        }
    }

    return sample;
}

float Oscillator::ProcessSample() {
    float sample = GenerateSample();

    // Advance phase
    phase_ += phaseIncrement_;
    phase_ = WrapPhase(phase_);

    return sample;
}

void Oscillator::Process(float* output, size_t numSamples) {
    assert(output != nullptr && "Output buffer cannot be null");

    for (size_t i = 0; i < numSamples; ++i) {
        output[i] = ProcessSample();
    }
}

} // namespace DSP
} // namespace DubSiren
