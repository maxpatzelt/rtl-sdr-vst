#include "LFO.h"
#include <cassert>
#include <cmath>

namespace DubSiren {
namespace DSP {

LFO::LFO()
    : sampleRate_  (44100.0f)
    , rate_        (1.0f)
    , amount_      (0.5f)
    , phase_       (0.0f)
    , phaseIncrement_(0.0f)
    , waveform_    (Waveform::Sine)
    , xstate_      (2463534242u)   // non-zero xorshift seed
    , shValue_     (0.0f)
    , prevPhase_   (0.0f)
{}

void LFO::Init(float sampleRate)
{
    assert(sampleRate > 0.0f);
    sampleRate_      = sampleRate;
    phaseIncrement_  = rate_ / sampleRate_;
    phase_           = 0.0f;
    prevPhase_       = 0.0f;
}

void LFO::SetRate(float rateHz)
{
    rate_            = Clamp(rateHz, 0.1f, 80.0f);
    phaseIncrement_  = rate_ / sampleRate_;
}

void LFO::SetAmount(float amount)
{
    amount_ = Clamp(amount, 0.0f, 1.0f);
}

void LFO::SetWaveform(Waveform w)
{
    waveform_ = w;
}

void LFO::Reset()
{
    phase_      = 0.0f;
    prevPhase_  = 0.0f;
}

float LFO::ProcessSample()
{
    float value = 0.0f;

    switch (waveform_)
    {
        case Waveform::Sine:
            value = std::sin(kTwoPi * phase_);
            break;

        case Waveform::Triangle:
            // 2·|2·(phase-floor(phase+0.5))| – 1  → bipolar triangle 0-peak at phase=0
            value = 4.0f * std::abs(phase_ - std::floor(phase_ + 0.5f)) - 1.0f;
            break;

        case Waveform::Square:
            value = (phase_ < 0.5f) ? 1.0f : -1.0f;
            break;

        case Waveform::SampleHold:
            // New random value each time phase wraps (rising zero-crossing)
            if (phase_ < prevPhase_)
            {
                xstate_ ^= xstate_ << 13;
                xstate_ ^= xstate_ >> 17;
                xstate_ ^= xstate_ << 5;
                shValue_ = (static_cast<float>(xstate_) / 4294967295.0f) * 2.0f - 1.0f;
            }
            value = shValue_;
            break;
    }

    prevPhase_  = phase_;
    phase_     += phaseIncrement_;
    phase_      = WrapPhase(phase_);

    return value;  // bipolar –1..+1
}

float LFO::GetModulationValue() const
{
    // Peek at current phase without advancing — only valid for non-S&H
    float lfoValue = std::sin(kTwoPi * phase_);
    return lfoValue * amount_;
}

} // namespace DSP
} // namespace DubSiren
