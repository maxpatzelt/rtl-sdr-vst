#pragma once

#include "Common.h"

namespace DubSiren {
namespace DSP {

/**
 * ADSR Envelope Generator
 *
 * Provides Attack-Decay-Sustain-Release amplitude envelope.
 * Linear segments for simplicity (exponential curves can be added later).
 *
 * States:
 * - Idle: Envelope is at 0, waiting for trigger
 * - Attack: Rising from 0 to 1
 * - Decay: Falling from 1 to sustain level
 * - Sustain: Holding at sustain level
 * - Release: Falling from current level to 0
 *
 * Design follows Mutable Instruments pattern: simple state machine with
 * clear stage transitions and sample-accurate timing.
 */
class Envelope {
public:
    enum class Stage {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    Envelope();
    ~Envelope() = default;

    /**
     * Initialize envelope with sample rate.
     */
    void Init(float sampleRate);

    /**
     * Set ADSR parameters.
     * attackMs: Attack time in milliseconds
     * decayMs: Decay time in milliseconds
     * sustainLevel: Sustain level (0.0 to 1.0)
     * releaseMs: Release time in milliseconds
     */
    void SetParameters(float attackMs, float decayMs,
                       float sustainLevel, float releaseMs);

    /**
     * Trigger the envelope (note on).
     * Begins attack stage from current level.
     */
    void NoteOn();

    /**
     * Release the envelope (note off).
     * Begins release stage from current level.
     */
    void NoteOff();

    /**
     * Reset envelope to idle state.
     */
    void Reset();

    /**
     * Process a single sample.
     * Returns current envelope level (0.0 to 1.0).
     */
    float ProcessSample();

    /**
     * Process a block of samples.
     */
    void Process(float* output, size_t numSamples);

    /**
     * Check if envelope is active (not idle or finished releasing).
     */
    bool IsActive() const;

    // Getters for testing
    Stage GetStage() const { return stage_; }
    float GetLevel() const { return level_; }

private:
    /**
     * Convert milliseconds to number of samples.
     */
    size_t MsToSamples(float ms) const;

    /**
     * Calculate increment per sample for a given stage duration.
     */
    float CalculateIncrement(float targetLevel, size_t durationSamples) const;

    float sampleRate_;

    // Current state
    Stage stage_;
    float level_;           // Current envelope output (0.0 to 1.0)
    size_t sampleCounter_;  // Samples processed in current stage

    // Parameters (stored as sample counts for efficiency)
    size_t attackSamples_;
    size_t decaySamples_;
    float sustainLevel_;    // 0.0 to 1.0
    size_t releaseSamples_;

    // Increments per sample for each stage
    float attackIncrement_;
    float decayIncrement_;
    float releaseIncrement_;
};

} // namespace DSP
} // namespace DubSiren
