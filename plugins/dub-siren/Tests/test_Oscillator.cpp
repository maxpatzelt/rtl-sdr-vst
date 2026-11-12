#include <juce_core/juce_core.h>
#include "DSP/Oscillator.h"

using namespace SimpleSynth::DSP;

/**
 * Oscillator Unit Tests
 *
 * Tests cover:
 * - Initialization and sample rate handling
 * - Frequency setting and clamping
 * - Waveform generation without NaN/Inf
 * - Phase continuity
 * - Zero-crossing validation for periodic signals
 */

class OscillatorTest : public juce::UnitTest {
public:
    OscillatorTest() : juce::UnitTest("Oscillator Tests") {}

    void runTest() override {
        beginTest("Initialization");
        testInitialization();

        beginTest("Frequency Setting");
        testFrequencySetting();

        beginTest("Waveform Output Validity");
        testWaveformOutputValidity();

        beginTest("Phase Continuity");
        testPhaseContinuity();

        beginTest("Sample Rate Change");
        testSampleRateChange();

        beginTest("Zero Crossings");
        testZeroCrossings();
    }

private:
    void testInitialization() {
        Oscillator osc;
        osc.Init(44100.0f);

        expect(osc.GetFrequency() == 440.0f, "Default frequency should be 440 Hz");
        expect(osc.GetPhase() == 0.0f, "Phase should initialize to 0");
        expect(osc.GetWaveform() == Oscillator::Waveform::Sine, "Default waveform should be Sine");
    }

    void testFrequencySetting() {
        Oscillator osc;
        osc.Init(44100.0f);

        // Valid frequency
        osc.SetFrequency(1000.0f);
        expectWithinAbsoluteError(osc.GetFrequency(), 1000.0f, 0.001f,
            "Frequency should be set to 1000 Hz");

        // Frequency below minimum should be clamped
        osc.SetFrequency(10.0f);
        expectGreaterOrEqual(osc.GetFrequency(), kMinFrequency,
            "Frequency below minimum should be clamped");

        // Frequency above Nyquist should be clamped
        osc.SetFrequency(25000.0f);
        expectLessThan(osc.GetFrequency(), 22050.0f,
            "Frequency above Nyquist should be clamped");
    }

    void testWaveformOutputValidity() {
        Oscillator osc;
        osc.Init(44100.0f);
        osc.SetFrequency(440.0f);

        const size_t bufferSize = 128;
        float buffer[bufferSize];

        // Test all waveforms
        std::array<Oscillator::Waveform, 3> waveforms = {
            Oscillator::Waveform::Sine,
            Oscillator::Waveform::Saw,
            Oscillator::Waveform::Square
        };

        for (auto waveform : waveforms) {
            osc.SetWaveform(waveform);
            osc.Reset();
            osc.Process(buffer, bufferSize);

            for (size_t i = 0; i < bufferSize; ++i) {
                expect(!std::isnan(buffer[i]), "Output should not be NaN");
                expect(!std::isinf(buffer[i]), "Output should not be Inf");
                expect(buffer[i] >= -1.5f && buffer[i] <= 1.5f,
                    "Output should be roughly in range [-1, 1]");
            }
        }
    }

    void testPhaseContinuity() {
        Oscillator osc;
        osc.Init(44100.0f);
        osc.SetFrequency(440.0f);
        osc.SetWaveform(Oscillator::Waveform::Sine);

        // Process samples and check phase advances
        float prev = osc.ProcessSample();

        for (int i = 0; i < 100; ++i) {
            float current = osc.ProcessSample();
            // Phase should advance continuously (samples should differ)
            // Note: We're not checking exact values, just that it's changing
        }

        // Phase should have wrapped at least once after enough samples
        float expectedPhaseIncrement = 440.0f / 44100.0f;
        float phaseAfter100Samples = WrapPhase(100.0f * expectedPhaseIncrement);
        expectWithinAbsoluteError(osc.GetPhase(), phaseAfter100Samples, 0.001f,
            "Phase should advance correctly");
    }

    void testSampleRateChange() {
        Oscillator osc;
        osc.Init(44100.0f);
        osc.SetFrequency(440.0f);

        // Change sample rate
        osc.Init(48000.0f);
        osc.SetFrequency(440.0f);

        // Should still produce valid output
        float sample = osc.ProcessSample();
        expect(!std::isnan(sample), "Output should be valid after sample rate change");
        expect(!std::isinf(sample), "Output should be finite after sample rate change");
    }

    void testZeroCrossings() {
        Oscillator osc;
        const float sampleRate = 44100.0f;
        const float frequency = 440.0f;
        osc.Init(sampleRate);
        osc.SetFrequency(frequency);
        osc.SetWaveform(Oscillator::Waveform::Sine);
        osc.Reset();

        // Count zero crossings over one second
        const size_t numSamples = static_cast<size_t>(sampleRate);
        int zeroCrossings = 0;
        float prevSample = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            float sample = osc.ProcessSample();

            // Detect zero crossing
            if ((prevSample < 0.0f && sample >= 0.0f) ||
                (prevSample >= 0.0f && sample < 0.0f)) {
                zeroCrossings++;
            }

            prevSample = sample;
        }

        // A sine wave at 440 Hz should have ~880 zero crossings per second
        // (2 per cycle). Allow 5% tolerance.
        int expectedCrossings = static_cast<int>(frequency * 2.0f);
        int tolerance = static_cast<int>(expectedCrossings * 0.05f);

        expectWithinAbsoluteError(static_cast<float>(zeroCrossings),
                                 static_cast<float>(expectedCrossings),
                                 static_cast<float>(tolerance),
                                 "Zero crossings should match expected frequency");
    }
};

static OscillatorTest oscillatorTest;
