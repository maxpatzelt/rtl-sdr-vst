#include "DubOscillator.h"
#include <cassert>
#include <cmath>

namespace DubSiren {
namespace DSP {

// ── PolyBLEP correction ───────────────────────────────────────────────────────
// Subtracts the BLEP from a naive waveform to remove the aliased step.
// t  = normalised phase in [0, 1)
// dt = phaseIncrement (normalised, = freq / sampleRate)
float DubOscillator::polyBlep(float t, float dt)
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    else if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

// ── xorshift32 PRNG ──────────────────────────────────────────────────────────
float DubOscillator::xorshiftNoise()
{
    xstate_ ^= xstate_ << 13;
    xstate_ ^= xstate_ >> 17;
    xstate_ ^= xstate_ << 5;
    return (static_cast<float>(xstate_) / 4294967295.0f) * 2.0f - 1.0f;
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────
DubOscillator::DubOscillator()
    : sampleRate_    (44100.0f)
    , frequency_     (440.0f)
    , level_         (0.8f)
    , phase_         (0.0f)
    , phaseIncrement_(0.0f)
    , waveform_      (Waveform::Square)
    , driftPhase_    (0.0f)
    , driftIncrement_(0.0f)
    , driftAmount_   (0.0015f)
    , xstate_        (1812433253u)   // non-zero seed
{}

void DubOscillator::Init(float sampleRate)
{
    assert(sampleRate > 0.0f);
    sampleRate_      = sampleRate;
    phaseIncrement_  = frequency_ / sampleRate_;
    // Drift oscillator ~2.7 Hz regardless of sample rate
    driftIncrement_  = 2.7f / sampleRate_;
    phase_           = 0.0f;
    driftPhase_      = 0.0f;
}

void DubOscillator::SetFrequency(float frequency)
{
    frequency_       = Clamp(frequency, kMinFrequency, kMaxFrequency);
    phaseIncrement_  = frequency_ / sampleRate_;
}

void DubOscillator::SetLevel(float level)
{
    level_ = Clamp(level, 0.0f, 1.0f);
}

void DubOscillator::SetWaveform(Waveform w)
{
    waveform_ = w;
}

void DubOscillator::Reset()
{
    phase_      = 0.0f;
    driftPhase_ = 0.0f;
}

// ── Core generator ────────────────────────────────────────────────────────────
float DubOscillator::GenerateSample()
{
    // Drift: slow sine modulation on phase for organic pitch instability
    driftPhase_ = WrapPhase(driftPhase_ + driftIncrement_);
    float drift = std::sin(kTwoPi * driftPhase_) * driftAmount_;

    // Effective phase includes drift
    float p   = WrapPhase(phase_ + drift);
    float dt  = phaseIncrement_;
    float out = 0.0f;

    switch (waveform_)
    {
        case Waveform::Square:
        {
            out  = (p < 0.5f) ? 1.0f : -1.0f;
            out += polyBlep(p,                    dt);   // rising edge at 0
            out -= polyBlep(WrapPhase(p + 0.5f),  dt);   // falling edge at 0.5
            break;
        }
        case Waveform::Saw:
        {
            out  = 2.0f * p - 1.0f;
            out -= polyBlep(p, dt);
            break;
        }
        case Waveform::Triangle:
        {
            // Integrate a square wave: naturally smooth, no polyBLEP needed
            out = 4.0f * std::abs(p - std::floor(p + 0.5f)) - 1.0f;
            break;
        }
    }

    // Tiny xorshift noise for that vintage transistor buzz (+/– 0.5 %)
    out += xorshiftNoise() * 0.005f;

    return out * level_;
}

float DubOscillator::ProcessSample()
{
    float sample = GenerateSample();
    phase_  += phaseIncrement_;
    phase_   = WrapPhase(phase_);
    return sample;
}

void DubOscillator::Process(float* output, size_t numSamples)
{
    assert(output != nullptr);
    for (size_t i = 0; i < numSamples; ++i)
        output[i] = ProcessSample();
}

} // namespace DSP
} // namespace DubSiren
