#pragma once

#include "Common.h"

namespace DubSiren {
namespace DSP {

/**
 * Band-limited oscillator with multiple waveforms.
 *
 * Generates sine, sawtooth, and square waveforms with anti-aliasing.
 * Uses PolyBLEP (Polynomial Band-Limited Step) for discontinuous waveforms
 * to reduce aliasing without lookup tables.
 *
 * Design inspired by Mutable Instruments' approach: clean separation between
 * state (phase, frequency) and rendering logic.
 */
class Oscillator {
public:
    enum class Waveform {
        Sine,
        Saw,
        Square
    };

    Oscillator();
    ~Oscillator() = default;

    /**
     * Initialize oscillator with sample rate.
     * Must be called before processing audio.
     */
    void Init(float sampleRate);

    /**
     * Set the oscillator frequency in Hz.
     * Clamped to valid audio range.
     */
    void SetFrequency(float frequency);

    /**
     * Set the waveform type.
     */
    void SetWaveform(Waveform waveform);

    /**
     * Reset phase to zero.
     * Useful for starting a note with consistent phase.
     */
    void Reset();

    /**
     * Process a single sample.
     * Returns the oscillator output in range [-1, 1].
     */
    float ProcessSample();

    /**
     * Process a block of samples.
     * output: Pointer to output buffer (must have space for numSamples)
     * numSamples: Number of samples to generate
     */
    void Process(float* output, size_t numSamples);

    // Getters for testing
    float GetFrequency() const { return frequency_; }
    float GetPhase() const { return phase_; }
    Waveform GetWaveform() const { return waveform_; }

private:
    /**
     * PolyBLEP residual for band-limiting discontinuities.
     * t: normalized distance from discontinuity (0 to 1)
     *
     * This polynomial smooths step discontinuities to reduce aliasing.
     * Applied at phase wraparound points in sawtooth and square waves.
     */
    float PolyBLEP(float t) const;

    /**
     * Generate one sample of the current waveform.
     */
    float GenerateSample();

    float sampleRate_;
    float frequency_;
    float phase_;           // Normalized phase [0, 1)
    float phaseIncrement_;  // Phase increment per sample
    Waveform waveform_;
};

} // namespace DSP
} // namespace DubSiren
