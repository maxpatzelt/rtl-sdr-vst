#include <juce_core/juce_core.h>
#include "DSP/Envelope.h"

using namespace SimpleSynth::DSP;

/**
 * Envelope Unit Tests
 *
 * Tests cover:
 * - ADSR stage transitions
 * - Attack reaches peak (1.0)
 * - Sustain holds correct level
 * - Release decays to zero
 * - Retrigger behavior
 * - Denormal prevention
 */

class EnvelopeTest : public juce::UnitTest {
public:
    EnvelopeTest() : juce::UnitTest("Envelope Tests") {}

    void runTest() override {
        beginTest("Initialization");
        testInitialization();

        beginTest("Attack Stage");
        testAttackStage();

        beginTest("Decay Stage");
        testDecayStage();

        beginTest("Sustain Stage");
        testSustainStage();

        beginTest("Release Stage");
        testReleaseStage();

        beginTest("Full ADSR Cycle");
        testFullCycle();

        beginTest("Retrigger During Release");
        testRetrigger();

        beginTest("No Denormals");
        testNoDenormals();
    }

private:
    void testInitialization() {
        Envelope env;
        env.Init(44100.0f);

        expect(env.GetStage() == Envelope::Stage::Idle, "Should start in Idle stage");
        expect(env.GetLevel() == 0.0f, "Should start at level 0");
        expect(!env.IsActive(), "Should not be active initially");
    }

    void testAttackStage() {
        Envelope env;
        env.Init(44100.0f);
        env.SetParameters(10.0f, 100.0f, 0.7f, 200.0f);  // 10ms attack

        env.NoteOn();
        expect(env.GetStage() == Envelope::Stage::Attack, "Should enter Attack stage");
        expect(env.IsActive(), "Should be active after note on");

        // Process attack phase (10ms at 44100 Hz = 441 samples)
        for (int i = 0; i < 441; ++i) {
            float level = env.ProcessSample();
            expect(level >= 0.0f && level <= 1.0f, "Level should be in valid range");
        }

        // Should now be in Decay or have reached peak
        expect(env.GetLevel() >= 0.99f, "Should reach near peak after attack time");
        expect(env.GetStage() == Envelope::Stage::Decay ||
               env.GetStage() == Envelope::Stage::Sustain,
               "Should transition from Attack");
    }

    void testDecayStage() {
        Envelope env;
        env.Init(44100.0f);
        env.SetParameters(1.0f, 50.0f, 0.5f, 100.0f);  // Fast attack, 50ms decay

        env.NoteOn();

        // Fast forward through attack
        for (int i = 0; i < 100; ++i) {
            env.ProcessSample();
        }

        // Should be in decay
        expect(env.GetStage() == Envelope::Stage::Decay, "Should be in Decay stage");

        float levelBeforeDecay = env.GetLevel();

        // Process decay
        for (int i = 0; i < 2205; ++i) {  // 50ms at 44100 Hz
            env.ProcessSample();
        }

        // Should have decayed to sustain level
        expectWithinAbsoluteError(env.GetLevel(), 0.5f, 0.05f,
            "Should reach sustain level after decay");
        expect(env.GetStage() == Envelope::Stage::Sustain, "Should be in Sustain stage");
    }

    void testSustainStage() {
        Envelope env;
        env.Init(44100.0f);
        env.SetParameters(1.0f, 10.0f, 0.6f, 100.0f);

        env.NoteOn();

        // Fast forward to sustain
        for (int i = 0; i < 1000; ++i) {
            env.ProcessSample();
        }

        expect(env.GetStage() == Envelope::Stage::Sustain, "Should be in Sustain");

        // Sustain should hold steady
        float sustainLevel = env.GetLevel();
        for (int i = 0; i < 1000; ++i) {
            float level = env.ProcessSample();
            expectWithinAbsoluteError(level, sustainLevel, 0.001f,
                "Sustain level should remain constant");
        }
    }

    void testReleaseStage() {
        Envelope env;
        env.Init(44100.0f);
        env.SetParameters(1.0f, 10.0f, 0.5f, 50.0f);  // 50ms release

        env.NoteOn();

        // Get to sustain
        for (int i = 0; i < 1000; ++i) {
            env.ProcessSample();
        }

        expect(env.GetStage() == Envelope::Stage::Sustain, "Should reach sustain");

        env.NoteOff();
        expect(env.GetStage() == Envelope::Stage::Release, "Should enter Release");

        float levelAtRelease = env.GetLevel();
        expect(levelAtRelease > 0.0f, "Should have non-zero level at release");

        // Process release (50ms at 44100 Hz = 2205 samples)
        for (int i = 0; i < 2205; ++i) {
            env.ProcessSample();
        }

        expectWithinAbsoluteError(env.GetLevel(), 0.0f, 0.01f,
            "Should reach zero after release");
        expect(env.GetStage() == Envelope::Stage::Idle, "Should return to Idle");
        expect(!env.IsActive(), "Should not be active after release");
    }

    void testFullCycle() {
        Envelope env;
        const float sampleRate = 44100.0f;
        env.Init(sampleRate);
        env.SetParameters(10.0f, 20.0f, 0.7f, 30.0f);

        // Note on
        env.NoteOn();
        expect(env.IsActive(), "Should be active");

        // Process for a while (100ms)
        for (int i = 0; i < 4410; ++i) {
            float level = env.ProcessSample();
            expect(!std::isnan(level), "Level should not be NaN");
            expect(!std::isinf(level), "Level should not be Inf");
            expect(level >= 0.0f && level <= 1.0f, "Level should be in range");
        }

        // Note off
        env.NoteOff();

        // Process release
        for (int i = 0; i < 2000; ++i) {
            env.ProcessSample();
        }

        expect(!env.IsActive(), "Should be inactive after complete cycle");
    }

    void testRetrigger() {
        Envelope env;
        env.Init(44100.0f);
        env.SetParameters(10.0f, 20.0f, 0.5f, 100.0f);

        // First note
        env.NoteOn();
        for (int i = 0; i < 1000; ++i) {
            env.ProcessSample();
        }

        // Release
        env.NoteOff();
        for (int i = 0; i < 500; ++i) {
            env.ProcessSample();
        }

        expect(env.GetStage() == Envelope::Stage::Release, "Should be releasing");
        float levelDuringRelease = env.GetLevel();
        expect(levelDuringRelease > 0.0f, "Should have non-zero level during release");

        // Retrigger during release
        env.NoteOn();
        expect(env.GetStage() == Envelope::Stage::Attack, "Should restart attack");

        // Should rise again
        for (int i = 0; i < 500; ++i) {
            env.ProcessSample();
        }

        expect(env.GetLevel() > levelDuringRelease,
            "Level should rise again after retrigger");
    }

    void testNoDenormals() {
        Envelope env;
        env.Init(44100.0f);
        env.SetParameters(1.0f, 1.0f, 0.0001f, 1000.0f);  // Very small sustain

        env.NoteOn();

        // Process to sustain
        for (int i = 0; i < 500; ++i) {
            env.ProcessSample();
        }

        env.NoteOff();

        // Process entire release
        for (int i = 0; i < 50000; ++i) {
            float level = env.ProcessSample();

            // Check that we don't get denormals (very small non-zero values)
            if (level != 0.0f) {
                expect(std::abs(level) >= kDenormalThreshold,
                    "Should prevent denormals");
            }
        }

        expect(env.GetLevel() == 0.0f, "Should reach exactly zero");
    }
};

static EnvelopeTest envelopeTest;
