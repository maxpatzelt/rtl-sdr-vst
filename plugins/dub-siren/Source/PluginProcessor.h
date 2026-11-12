#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/DubOscillator.h"
#include "DSP/LFO.h"
#include "DSP/DubDelay.h"
#include "DSP/Envelope.h"
#include "DSP/SamplePlayer.h"

/**
 * Dub Siren VST Processor  v2.0
 *
 * Classic dub siren synthesizer with:
 *  - Band-limited VCO (Square / Saw / Triangle)  with polyBLEP
 *  - Portamento / glide
 *  - Two LFOs (Sine / Tri / Square / S+H)  with full modulation routing
 *  - Dub-style delay effect
 *  - Stereo output
 *
 * Audio thread rules:
 *  - Zero heap allocation in processBlock
 *  - All APVTS parameter reads cached ONCE at block boundary
 *  - No rand() — xorshift32 PRNG used in DSP classes
 */
class DubSirenProcessor : public juce::AudioProcessor
{
public:
    DubSirenProcessor();
    ~DubSirenProcessor() override;

    //==========================================================================
    void prepareToPlay  (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock   (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool  acceptsMidi()   const override;
    bool  producesMidi()  const override;
    bool  isMidiEffect()  const override;
    double getTailLengthSeconds() const override;

    int   getNumPrograms() override;
    int   getCurrentProgram() override;
    void  setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void  changeProgramName(int, const juce::String&) override;

    void  getStateInformation(juce::MemoryBlock&) override;
    void  setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState& getParameters() { return parameters_; }

    void triggerButtonOn();
    void triggerButtonOff();

    void sampleButtonTrigger();
    void selectSample(int index);
    static constexpr int getNumSamples() { return 14; }

    // LFO routing enums — must match choice parameter order in createParameterLayout()
    enum class LFO1Target { None = 0, VCORate, Delay, SampleRate, Filter };
    enum class LFO2Target { None = 0, LFO1Rate, LFO1Amount, Delay, SampleRate };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP modules
    DubSiren::DSP::DubOscillator dubOscillator_;
    DubSiren::DSP::LFO           lfo1_;
    DubSiren::DSP::LFO           lfo2_;
    DubSiren::DSP::DubDelay      dubDelay_;
    DubSiren::DSP::Envelope      envelope_;
    DubSiren::DSP::SamplePlayer  samplePlayer_;

    // Decoded sample buffers — populated once in prepareToPlay
    static constexpr int NUM_SAMPLES = 14;
    juce::AudioBuffer<float>     sampleBuffers_[NUM_SAMPLES];
    double                       sampleSourceRates_[NUM_SAMPLES] = {};
    int                          currentSampleIdx_ = 0;

    // Parameter management
    juce::AudioProcessorValueTreeState parameters_;

    // MIDI state
    int   currentMidiNote_    = -1;
    bool  isNoteOn_           = false;
    float currentNoteVelocity_= 1.0f;

    // Portamento (1-pole slew on VCO frequency)
    float slewedVCOFreq_      = 440.0f;

    // State-variable filter (Chamberlin SVF) — VCO signal path only
    float filterLow_          = 0.0f;
    float filterBand_         = 0.0f;
    float currentFilterCutoff_= 3500.0f;
    bool  filterDecaying_     = false;

    // Button trigger — written from UI thread, consumed in audio thread
    std::atomic<bool> buttonNoteOnPending_  { false };
    std::atomic<bool> buttonNoteOffPending_ { false };
    std::atomic<bool> sampleTriggerPending_  { false };
    std::atomic<int>  sampleSelectPending_   { -1 };   // -1 = no change
    std::atomic<bool> sampleLooping_         { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DubSirenProcessor)
};
