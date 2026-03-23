/*
  RTL-SDR Plugin Processor Implementation v3.0

  v3.0 changes:
    - Added AGC, squelch, tape saturation, fine-tune parameters
    - Pre-allocated processing buffers (no heap alloc in processBlock)
    - Per-sample AGC and soft-saturation DSP
    - Squelch with exponential envelope follower
    - Fine-tune offset applied to SDR hardware frequency
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

RTLSDRPluginProcessor::RTLSDRPluginProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // ── Core parameters ───────────────────────────────────────────────────────
    addParameter (frequencyParam = new juce::AudioParameterFloat (
        juce::ParameterID { "frequency", 1 }, "Frequency (MHz)",
        juce::NormalisableRange<float> (24.0f, 1766.0f, 0.001f), 100.0f));

    addParameter (gainParam = new juce::AudioParameterFloat (
        juce::ParameterID { "gain", 1 }, "Gain (dB)",
        juce::NormalisableRange<float> (0.0f, 50.0f, 0.1f), 29.0f));

    addParameter (modeParam = new juce::AudioParameterChoice (
        juce::ParameterID { "mode", 1 }, "Mode",
        juce::StringArray { "FM (Wideband)", "FM (Narrowband)", "AM", "USB", "LSB" }, 0));

    addParameter (volumeParam = new juce::AudioParameterFloat (
        juce::ParameterID { "volume", 1 }, "Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.7f));

    // ── v3.0 parameters ───────────────────────────────────────────────────────
    addParameter (agcEnabledParam = new juce::AudioParameterBool (
        juce::ParameterID { "agcEnabled", 1 }, "AGC", false));

    addParameter (squelchParam = new juce::AudioParameterFloat (
        juce::ParameterID { "squelch", 1 }, "Squelch",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    addParameter (saturationParam = new juce::AudioParameterFloat (
        juce::ParameterID { "saturation", 1 }, "Saturation",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    addParameter (fineTuneParam = new juce::AudioParameterFloat (
        juce::ParameterID { "fineTune", 1 }, "Fine Tune (kHz)",
        juce::NormalisableRange<float> (-25.0f, 25.0f, 0.1f), 0.0f));
}

RTLSDRPluginProcessor::~RTLSDRPluginProcessor()
{
    stopSDR();
}

//==============================================================================
const juce::String RTLSDRPluginProcessor::getName() const { return JucePlugin_Name; }
bool RTLSDRPluginProcessor::acceptsMidi()  const { return false; }
bool RTLSDRPluginProcessor::producesMidi() const { return false; }
bool RTLSDRPluginProcessor::isMidiEffect() const { return false; }
double RTLSDRPluginProcessor::getTailLengthSeconds() const { return 0.0; }
int RTLSDRPluginProcessor::getNumPrograms() { return 1; }
int RTLSDRPluginProcessor::getCurrentProgram() { return 0; }
void RTLSDRPluginProcessor::setCurrentProgram (int) {}
const juce::String RTLSDRPluginProcessor::getProgramName (int) { return {}; }
void RTLSDRPluginProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void RTLSDRPluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    hostSampleRate = sampleRate;

    // ── Configure DSP chain ──────────────────────────────────────────────────
    fmDemod.setSampleRate (1024000.0);
    fmDemod.setAudioSampleRate (sampleRate);
    fmDemod.setBandwidth (150000.0f);

    resampler.setRates (1024000.0, sampleRate);

    iqBuffer.clear();
    audioBuffer.clear();

    // ── Pre-allocate all processing buffers ───────────────────────────────────
    // Reserve capacity to max expected sizes.  After this point, push_back()
    // inside demodulators will not trigger heap allocation (as long as we
    // don't exceed capacity).
    tempIQBuffer.clear();
    tempIQBuffer.reserve (MAX_IQ_BLOCK);
    tempIQBuffer.resize  (MAX_IQ_BLOCK, 0);

    tempAudioBuffer.clear();
    tempAudioBuffer.reserve (MAX_AUDIO_BLOCK);

    resampledAudioBuffer.clear();
    resampledAudioBuffer.reserve (MAX_RESAMP_OUT);

    outputSamplesBuffer.clear();
    outputSamplesBuffer.resize (juce::jmax (samplesPerBlock, MAX_DAW_BLOCK), 0.0f);

    // ── AGC time constants (computed at SDR rate, which is fixed at 1024 kHz) ─
    const float sdrRate = 1024000.0f;
    agcAttackCoeff  = std::exp (-1.0f / (5.0f   * 0.001f * sdrRate));   //   5 ms
    agcReleaseCoeff = std::exp (-1.0f / (150.0f  * 0.001f * sdrRate));   // 150 ms
    agcEnvelope     = 0.0f;

    // ── Squelch envelope time constant (measured at host rate) ────────────────
    squelchEnvCoeff = std::exp (-1.0f / (50.0f * 0.001f * (float)sampleRate));
    signalEnvelope  = 0.0f;
}

void RTLSDRPluginProcessor::releaseResources()
{
    stopSDR();
}

bool RTLSDRPluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono()
        || out == juce::AudioChannelSet::stereo();
}

//==============================================================================
void RTLSDRPluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int totalIn  = getTotalNumInputChannels();
    const int totalOut = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (auto i = totalIn; i < totalOut; ++i)
        buffer.clear (i, 0, numSamples);

    // ── Update hardware when parameters change ─────────────────────────────
    const float freqMHz    = frequencyParam->get();
    const float fineTuneKHz = fineTuneParam ? fineTuneParam->get() : 0.0f;
    const float actualFreq = freqMHz + fineTuneKHz * 0.001f;  // kHz → MHz

    if (std::abs (actualFreq - currentFrequency.load()) > 0.0005f)
    {
        currentFrequency.store (actualFreq);
        sdrDevice.setFrequency ((uint32_t)(actualFreq * 1e6f));
    }

    const float gainDb = gainParam->get();
    if (std::abs (gainDb - currentGain.load()) > 0.1f)
    {
        currentGain.store (gainDb);
        sdrDevice.setGain ((int)(gainDb * 10.0f));
    }

    currentVolume.store (volumeParam->get());
    currentMode.store   (modeParam->getIndex());

    // ── Demodulate IQ → audio ──────────────────────────────────────────────
    size_t iqAvailable = iqBuffer.getReadAvailable();

    if (iqAvailable > 0)
    {
        // Read into pre-allocated buffer — no resize here
        size_t toRead = std::min (iqAvailable, (size_t)MAX_IQ_BLOCK);
        iqBuffer.read (tempIQBuffer.data(), toRead);

        // clear() sets size to 0 without freeing capacity — safe on audio thread
        tempAudioBuffer.clear();

        const int mode = currentMode.load();
        if (mode == 2)
        {
            amDemod.processBlock (tempIQBuffer.data(), (int)toRead, tempAudioBuffer);
        }
        else if (mode == 3)
        {
            ssbDemod.setMode (SSBDemodulator::USB);
            ssbDemod.processBlock (tempIQBuffer.data(), (int)toRead, tempAudioBuffer);
        }
        else if (mode == 4)
        {
            ssbDemod.setMode (SSBDemodulator::LSB);
            ssbDemod.processBlock (tempIQBuffer.data(), (int)toRead, tempAudioBuffer);
        }
        else
        {
            fmDemod.processIQ (tempIQBuffer.data(), (int)toRead, tempAudioBuffer);
        }

        // ── AGC ──────────────────────────────────────────────────────────────
        if (agcEnabledParam && agcEnabledParam->get() && !tempAudioBuffer.empty())
            applyAGC (tempAudioBuffer.data(), (int)tempAudioBuffer.size());

        // ── Resample → host rate ──────────────────────────────────────────────
        resampledAudioBuffer.clear();  // capacity preserved — no allocation
        resampler.process (tempAudioBuffer, resampledAudioBuffer);

        // ── Tape saturation ───────────────────────────────────────────────────
        const float sat = saturationParam ? saturationParam->get() : 0.0f;
        if (sat > 0.01f && !resampledAudioBuffer.empty())
            applySaturation (resampledAudioBuffer.data(), (int)resampledAudioBuffer.size(), sat);

        // ── Update squelch envelope (from resampled audio) ────────────────────
        if (!resampledAudioBuffer.empty())
            updateSignalEnvelope (resampledAudioBuffer.data(), (int)resampledAudioBuffer.size());

        // Write to output ring buffer
        if (!resampledAudioBuffer.empty())
            audioBuffer.write (resampledAudioBuffer.data(), resampledAudioBuffer.size());
    }

    // ── Squelch gate ──────────────────────────────────────────────────────────
    const float squelchThreshold = squelchParam ? squelchParam->get() : 0.0f;
    const bool squelched = (squelchThreshold > 0.02f) && !isAboveSquelch (squelchThreshold);

    // ── Fill DAW output buffer ────────────────────────────────────────────────
    const size_t audioAvailable = audioBuffer.getReadAvailable();

    if (!squelched && audioAvailable >= (size_t)numSamples)
    {
        // Use pre-allocated output buffer — no local vector
        audioBuffer.read (outputSamplesBuffer.data(), numSamples);

        const float volume = currentVolume.load();
        for (int ch = 0; ch < totalOut; ++ch)
        {
            auto* out = buffer.getWritePointer (ch);
            for (int i = 0; i < numSamples; ++i)
                out[i] = outputSamplesBuffer[i] * volume;
        }
    }
    else
    {
        buffer.clear();
        if (squelched)
            audioBuffer.clear();  // drain accumulated audio while squelched
    }
}

//==============================================================================
// DSP helpers — all in-place, no allocation

void RTLSDRPluginProcessor::applyAGC (float* samples, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        const float level = std::abs (samples[i]);

        // Asymmetric envelope follower: fast attack, slow release
        if (level > agcEnvelope)
            agcEnvelope = agcAttackCoeff  * agcEnvelope + (1.0f - agcAttackCoeff)  * level;
        else
            agcEnvelope = agcReleaseCoeff * agcEnvelope + (1.0f - agcReleaseCoeff) * level;

        agcEnvelope += 1e-25f;  // denormal prevention

        if (agcEnvelope > 1e-4f)
            samples[i] *= AGC_TARGET / agcEnvelope;

        // Hard-limit to avoid spikes on sudden strong signals
        samples[i] = juce::jlimit (-1.0f, 1.0f, samples[i]);
    }
}

void RTLSDRPluginProcessor::applySaturation (float* samples, int numSamples, float amount)
{
    // Soft knee saturation via tanh — preserves unity gain at centre
    const float drive      = 1.0f + amount * 8.0f;
    const float driveRecip = 1.0f / std::tanh (drive);

    for (int i = 0; i < numSamples; ++i)
        samples[i] = std::tanh (samples[i] * drive) * driveRecip;
}

void RTLSDRPluginProcessor::updateSignalEnvelope (const float* samples, int numSamples)
{
    // Compute RMS of first N samples (capped at 256 for efficiency)
    const int count = std::min (numSamples, 256);
    float sumSq = 0.0f;

    for (int i = 0; i < count; ++i)
        sumSq += samples[i] * samples[i];

    const float rms = std::sqrt (sumSq / (float)count + 1e-25f);

    // Smooth with single-pole IIR
    signalEnvelope = squelchEnvCoeff * signalEnvelope + (1.0f - squelchEnvCoeff) * rms;
    signalEnvelope += 1e-25f;
}

bool RTLSDRPluginProcessor::isAboveSquelch (float threshold) const
{
    // Map threshold 0-1 to a squared RMS level (more natural taper)
    const float squelchLevel = threshold * threshold * 0.5f;
    return signalEnvelope > squelchLevel;
}

//==============================================================================
bool RTLSDRPluginProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* RTLSDRPluginProcessor::createEditor()
{
    return new RTLSDRPluginEditor (*this);
}

//==============================================================================
void RTLSDRPluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream (destData, true);
    // v1/v2 params
    stream.writeFloat (frequencyParam->get());
    stream.writeFloat (gainParam->get());
    stream.writeInt   (modeParam->getIndex());
    stream.writeFloat (volumeParam->get());
    // v3 params
    stream.writeBool  (agcEnabledParam ? (bool)agcEnabledParam->get() : false);
    stream.writeFloat (squelchParam    ? squelchParam->get()    : 0.0f);
    stream.writeFloat (saturationParam ? saturationParam->get() : 0.0f);
    stream.writeFloat (fineTuneParam   ? fineTuneParam->get()   : 0.0f);
}

void RTLSDRPluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream (data, static_cast<size_t>(sizeInBytes), false);

    auto setFloat = [&](juce::AudioParameterFloat* p)
    {
        if (p && stream.getNumBytesRemaining() >= 4)
            p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (stream.readFloat()));
    };
    auto setInt = [&](juce::AudioParameterChoice* p)
    {
        if (p && stream.getNumBytesRemaining() >= 4)
            p->setValueNotifyingHost ((float)stream.readInt() / (float)(p->choices.size() - 1));
    };
    auto setBool = [&](juce::AudioParameterBool* p)
    {
        if (p && stream.getNumBytesRemaining() >= 1)
            p->setValueNotifyingHost (stream.readBool() ? 1.0f : 0.0f);
    };

    setFloat (frequencyParam);
    setFloat (gainParam);
    setInt   (modeParam);
    setFloat (volumeParam);
    setBool  (agcEnabledParam);
    setFloat (squelchParam);
    setFloat (saturationParam);
    setFloat (fineTuneParam);
}

//==============================================================================
bool RTLSDRPluginProcessor::startSDR (int deviceIndex)
{
    if (!sdrDevice.openDevice (deviceIndex))
        return false;

    const float actualFreq = currentFrequency.load();
    sdrDevice.setFrequency  ((uint32_t)(actualFreq * 1e6f));
    sdrDevice.setSampleRate (1024000);
    sdrDevice.setAutoGain   (false);
    sdrDevice.setGain       ((int)(currentGain.load() * 10.0f));

    return sdrDevice.startStreaming ([this](const uint8_t* d, uint32_t len)
    {
        onSDRSamples (d, len);
    });
}

void RTLSDRPluginProcessor::stopSDR()
{
    sdrDevice.stopStreaming();
    sdrDevice.closeDevice();
}

std::vector<RTLSDRDevice::DeviceInfo> RTLSDRPluginProcessor::getAvailableDevices()
{
    return sdrDevice.enumerateDevices();
}

void RTLSDRPluginProcessor::onSDRSamples (const uint8_t* data, uint32_t length)
{
    // SDR capture thread — just buffer the raw IQ bytes
    iqBuffer.write (data, length);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RTLSDRPluginProcessor();
}
