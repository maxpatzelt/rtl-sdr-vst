/*
  RTL-SDR VST3 Plugin Processor v3.0
  Bridges RTL-SDR hardware to a DAW for capturing radio signals as audio.

  Thread model:
    SDR thread  → writes IQ samples to iqBuffer (lock-free)
    Audio thread← reads iqBuffer, demodulates, resamples → audioBuffer → DAW

  Audio thread rules (non-negotiable):
    - No heap allocation in processBlock (all buffers pre-allocated in prepareToPlay)
    - No mutexes / blocking
    - Denormal prevention on all recursive filters
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "RTLSDRDevice.h"
#include "FMDemodulator.h"
#include "AMDemodulator.h"
#include "SSBDemodulator.h"
#include "AudioResampler.h"
#include "RingBuffer.h"

class RTLSDRPluginProcessor : public juce::AudioProcessor
{
public:
    RTLSDRPluginProcessor();
    ~RTLSDRPluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi()  const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    // RTL-SDR control
    bool startSDR (int deviceIndex = 0);
    void stopSDR();
    bool isSDRRunning() const { return sdrDevice.isStreaming(); }

    std::vector<RTLSDRDevice::DeviceInfo> getAvailableDevices();

    // Expose IQ buffer for visualizers (read from timer thread)
    RingBuffer<uint8_t>& getIQBuffer() { return iqBuffer; }

    //==========================================================================
    // Parameters — owned by the AudioProcessor, exposed for UI access
    juce::AudioParameterFloat*  frequencyParam  = nullptr;  // 24 – 1766 MHz
    juce::AudioParameterFloat*  gainParam       = nullptr;  // 0 – 50 dB
    juce::AudioParameterChoice* modeParam       = nullptr;  // FM WB/NB, AM, USB, LSB
    juce::AudioParameterFloat*  volumeParam     = nullptr;  // 0 – 1

    // v3.0 additions
    juce::AudioParameterBool*   agcEnabledParam = nullptr;  // Automatic gain control
    juce::AudioParameterFloat*  squelchParam    = nullptr;  // 0 = open, 1 = closed
    juce::AudioParameterFloat*  saturationParam = nullptr;  // 0 = clean, 1 = saturated
    juce::AudioParameterFloat*  fineTuneParam   = nullptr;  // -25 to +25 kHz offset

private:
    //==========================================================================
    // SDR components
    RTLSDRDevice    sdrDevice;
    FMDemodulator   fmDemod;
    AMDemodulator   amDemod;
    SSBDemodulator  ssbDemod;
    AudioResampler  resampler;
    RingBuffer<uint8_t> iqBuffer;
    RingBuffer<float>   audioBuffer;

    // Parameter shadows (atomic for lock-free cross-thread access)
    std::atomic<float> currentFrequency { 100.0f };  // MHz
    std::atomic<float> currentGain      { 29.0f };   // dB
    std::atomic<float> currentVolume    { 0.7f };
    std::atomic<int>   currentMode      { 0 };

    double hostSampleRate = 48000.0;

    //==========================================================================
    // Pre-allocated processing buffers — sized in prepareToPlay, NEVER resized
    // in processBlock.  Having capacity pre-reserved means push_back()
    // inside the demodulators cannot trigger a heap allocation.
    static constexpr int MAX_IQ_BLOCK    = 65536;  // max IQ bytes read per block
    static constexpr int MAX_AUDIO_BLOCK = 32768;  // max demodulated samples
    static constexpr int MAX_RESAMP_OUT  = 8192;   // max resampled output samples
    static constexpr int MAX_DAW_BLOCK   = 4096;   // max DAW output block

    std::vector<uint8_t> tempIQBuffer;
    std::vector<float>   tempAudioBuffer;
    std::vector<float>   resampledAudioBuffer;
    std::vector<float>   outputSamplesBuffer;

    //==========================================================================
    // AGC state
    float agcEnvelope     = 0.0f;
    float agcAttackCoeff  = 0.995f;   // re-computed in prepareToPlay
    float agcReleaseCoeff = 0.9999f;  // re-computed in prepareToPlay
    static constexpr float AGC_TARGET = 0.5f;

    // Squelch envelope tracker
    float signalEnvelope   = 0.0f;
    float squelchEnvCoeff  = 0.99f;   // re-computed in prepareToPlay

    //==========================================================================
    // Audio-thread DSP helpers (no allocation, pure in-place)
    void applyAGC        (float* samples, int numSamples);
    void applySaturation (float* samples, int numSamples, float amount);
    void updateSignalEnvelope (const float* samples, int numSamples);
    bool isAboveSquelch  (float threshold) const;

    // SDR capture callback (SDR thread)
    void onSDRSamples (const uint8_t* data, uint32_t length);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RTLSDRPluginProcessor)
};
