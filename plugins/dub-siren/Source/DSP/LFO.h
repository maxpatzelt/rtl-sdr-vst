#pragma once

#include "Common.h"
#include <cstdint>

namespace DubSiren {
namespace DSP {

/**
 * Low Frequency Oscillator
 *
 * Free-running modulation source with multiple waveforms.
 * Sine / Triangle / Square / Sample-and-Hold.
 * S+H uses an internal xorshift32 PRNG — no rand(), fully deterministic.
 */
class LFO {
public:
    enum class Waveform { Sine = 0, Triangle, Square, SampleHold };

    LFO();
    ~LFO() = default;

    void Init(float sampleRate);
    void SetRate(float rateHz);        // LFO frequency in Hz
    void SetAmount(float amount);      // Modulation depth 0.0–1.0
    void SetWaveform(Waveform w);
    void Reset();

    float ProcessSample();             // Bipolar –1.0 to +1.0
    float GetModulationValue() const;  // Scaled by amount

private:
    float   sampleRate_;
    float   rate_;
    float   amount_;
    float   phase_;
    float   phaseIncrement_;
    Waveform waveform_;

    // S+H state
    uint32_t xstate_;   // xorshift32 seed
    float    shValue_;  // currently held value
    float    prevPhase_;
};

} // namespace DSP
} // namespace DubSiren
