#pragma once

#include "Common.h"
#include <cstdint>

namespace DubSiren {
namespace DSP {

/**
 * Dub Siren Oscillator
 *
 * Band-limited VCO with PolyBLEP anti-aliasing on square and sawtooth edges.
 * Triangle is computed directly (already band-limited).
 * Analog drift uses a low-frequency sine on a separate accumulator
 * (sample-rate-aware — no hardcoded rate).
 * Noise uses xorshift32 PRNG — no rand(), fully deterministic and thread-safe.
 */
class DubOscillator {
public:
    enum class Waveform { Square = 0, Saw, Triangle };

    DubOscillator();
    ~DubOscillator() = default;

    void  Init      (float sampleRate);
    void  SetFrequency (float frequency);
    void  SetLevel  (float level);        // 0.0–1.0
    void  SetWaveform  (Waveform w);
    void  Reset();

    float ProcessSample();
    void  Process(float* output, size_t numSamples);

private:
    float GenerateSample();

    // PolyBLEP helper — bandlimits a step discontinuity
    static float polyBlep(float t, float dt);

    // xorshift32 — returns bipolar noise in [–1, +1]
    float xorshiftNoise();

    float    sampleRate_;
    float    frequency_;
    float    level_;
    float    phase_;
    float    phaseIncrement_;
    Waveform waveform_;

    // Analog drift — separate slow oscillator at ~3 Hz relative to sampleRate
    float    driftPhase_;
    float    driftIncrement_;  // recalculated in Init / SetFrequency
    float    driftAmount_;

    uint32_t xstate_;          // xorshift32 state (non-zero)
};

} // namespace DSP
} // namespace DubSiren
