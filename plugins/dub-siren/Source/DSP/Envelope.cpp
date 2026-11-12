#include "Envelope.h"
#include <cassert>

namespace DubSiren {
namespace DSP {

Envelope::Envelope()
    : sampleRate_(44100.0f)
    , stage_(Stage::Idle)
    , level_(0.0f)
    , sampleCounter_(0)
    , attackSamples_(0)
    , decaySamples_(0)
    , sustainLevel_(0.7f)
    , releaseSamples_(0)
    , attackIncrement_(0.0f)
    , decayIncrement_(0.0f)
    , releaseIncrement_(0.0f)
{
    // Set reasonable defaults: 10ms attack, 100ms decay, 0.7 sustain, 200ms release
    SetParameters(10.0f, 100.0f, 0.7f, 200.0f);
}

void Envelope::Init(float sampleRate) {
    assert(sampleRate > 0.0f && "Sample rate must be positive");
    sampleRate_ = sampleRate;

    // Recalculate all time-based parameters
    attackSamples_ = MsToSamples(attackSamples_ * 1000.0f / sampleRate_);
    decaySamples_ = MsToSamples(decaySamples_ * 1000.0f / sampleRate_);
    releaseSamples_ = MsToSamples(releaseSamples_ * 1000.0f / sampleRate_);

    Reset();
}

void Envelope::SetParameters(float attackMs, float decayMs,
                              float sustainLevel, float releaseMs) {
    // Clamp parameters to valid ranges
    attackMs = std::max(0.1f, attackMs);      // Minimum 0.1ms to avoid divide by zero
    decayMs = std::max(0.1f, decayMs);
    sustainLevel = Clamp(sustainLevel, 0.0f, 1.0f);
    releaseMs = std::max(0.1f, releaseMs);

    attackSamples_ = MsToSamples(attackMs);
    decaySamples_ = MsToSamples(decayMs);
    sustainLevel_ = sustainLevel;
    releaseSamples_ = MsToSamples(releaseMs);

    // Precalculate increments for linear interpolation
    attackIncrement_ = CalculateIncrement(1.0f, attackSamples_);
    decayIncrement_ = CalculateIncrement(sustainLevel_, decaySamples_);
    // Release increment is calculated dynamically based on current level
}

void Envelope::NoteOn() {
    stage_ = Stage::Attack;
    sampleCounter_ = 0;

    // If attack time is zero, jump directly to decay
    if (attackSamples_ == 0) {
        level_ = 1.0f;
        stage_ = Stage::Decay;
    }
}

void Envelope::NoteOff() {
    stage_ = Stage::Release;
    sampleCounter_ = 0;

    // Calculate release increment from current level
    if (releaseSamples_ > 0) {
        releaseIncrement_ = -level_ / static_cast<float>(releaseSamples_);
    } else {
        releaseIncrement_ = -level_;  // Instant release
    }
}

void Envelope::Reset() {
    stage_ = Stage::Idle;
    level_ = 0.0f;
    sampleCounter_ = 0;
}

size_t Envelope::MsToSamples(float ms) const {
    return static_cast<size_t>(ms * sampleRate_ / 1000.0f);
}

float Envelope::CalculateIncrement(float targetLevel, size_t durationSamples) const {
    if (durationSamples == 0) {
        return targetLevel - level_;  // Instant change
    }
    return (targetLevel - level_) / static_cast<float>(durationSamples);
}

float Envelope::ProcessSample() {
    switch (stage_) {
        case Stage::Idle:
            level_ = 0.0f;
            break;

        case Stage::Attack:
            level_ += attackIncrement_;
            sampleCounter_++;

            if (sampleCounter_ >= attackSamples_ || level_ >= 1.0f) {
                level_ = 1.0f;
                stage_ = Stage::Decay;
                sampleCounter_ = 0;

                // Recalculate decay increment from peak
                decayIncrement_ = (sustainLevel_ - 1.0f) /
                                  static_cast<float>(std::max(size_t(1), decaySamples_));
            }
            break;

        case Stage::Decay:
            level_ += decayIncrement_;
            sampleCounter_++;

            if (sampleCounter_ >= decaySamples_ || level_ <= sustainLevel_) {
                level_ = sustainLevel_;
                stage_ = Stage::Sustain;
                sampleCounter_ = 0;
            }
            break;

        case Stage::Sustain:
            level_ = sustainLevel_;
            break;

        case Stage::Release:
            level_ += releaseIncrement_;
            sampleCounter_++;

            if (sampleCounter_ >= releaseSamples_ || level_ <= 0.0f) {
                level_ = 0.0f;
                stage_ = Stage::Idle;
                sampleCounter_ = 0;
            }
            break;
    }

    // Clamp to valid range and prevent denormals
    level_ = Clamp(level_, 0.0f, 1.0f);
    level_ = PreventDenormal(level_);

    return level_;
}

void Envelope::Process(float* output, size_t numSamples) {
    assert(output != nullptr && "Output buffer cannot be null");

    for (size_t i = 0; i < numSamples; ++i) {
        output[i] = ProcessSample();
    }
}

bool Envelope::IsActive() const {
    return stage_ != Stage::Idle;
}

} // namespace DSP
} // namespace DubSiren
