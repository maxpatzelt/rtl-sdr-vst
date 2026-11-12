#pragma once

#include "Common.h"
#include <vector>

namespace DubSiren {
namespace DSP {

/**
 * Dub Delay Effect
 *
 * Classic reggae-style delay with analog character.
 * Circular buffer implementation with feedback and wet/dry mix.
 * Adds slight instability for organic feel.
 */
class DubDelay {
public:
    DubDelay();
    ~DubDelay() = default;

    void Init(float sampleRate, float maxDelayTimeSeconds = 2.0f);
    void SetDelayTime(float timeSeconds);
    void SetFeedback(float feedback); // 0.0 to 0.95
    void SetWetDry(float wetDry); // 0.0 = dry, 1.0 = wet
    void Reset();

    float ProcessSample(float input);
    void Process(float* buffer, size_t numSamples);

private:
    float sampleRate_;
    std::vector<float> delayBuffer_;
    size_t bufferSize_;
    size_t writeIndex_;

    float delayTimeSeconds_;
    float feedback_;
    float wetDry_;

    // Analog instability
    float wobblePhase_;
    float wobbleAmount_;
};

} // namespace DSP
} // namespace DubSiren
