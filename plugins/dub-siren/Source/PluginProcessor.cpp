#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <cmath>
#include <atomic>
#include <BinaryData.h>

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
DubSirenProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // ── VCO ────────────────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "vcoRate", "VCO Rate",
        juce::NormalisableRange<float>(30.0f, 9000.0f, 0.1f, 0.3f), 440.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "vcoLevel", "VCO Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "vcoWaveform", "VCO Waveform",
        juce::StringArray{ "Square", "Saw", "Triangle" }, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "portamento", "Portamento",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // ── Delay ──────────────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayTime", "Delay Time",
        juce::NormalisableRange<float>(0.05f, 2.0f, 0.001f, 0.5f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayFeedback", "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.99f), 0.55f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayWetDry", "Delay Wet/Dry",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.45f));

    // ── LFO 1 ──────────────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo1Rate", "LFO 1 Rate",
        juce::NormalisableRange<float>(0.05f, 12.0f, 0.01f, 0.5f), 1.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo1Amount", "LFO 1 Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "lfo1Target", "LFO 1 Target",
        juce::StringArray{ "None", "VCO Rate", "Delay", "Sample Rate", "Filter" }, 0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "lfo1Waveform", "LFO 1 Waveform",
        juce::StringArray{ "Sine", "Tri", "Square", "S+H" }, 0));

    // ── LFO 2 ──────────────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo2Rate", "LFO 2 Rate",
        juce::NormalisableRange<float>(0.05f, 12.0f, 0.01f, 0.5f), 0.3f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo2Amount", "LFO 2 Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "lfo2Target", "LFO 2 Target",
        juce::StringArray{ "None", "LFO1 Rate", "LFO1 Amount", "Delay", "Sample Rate" }, 0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "lfo2Waveform", "LFO 2 Waveform",
        juce::StringArray{ "Sine", "Tri", "Square", "S+H" }, 0));

    // ── Sample ─────────────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "sampleLevel", "Sample Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "samplePlayRate", "Sample Play Rate",
        juce::NormalisableRange<float>(0.1f, 4.0f, 0.01f, 0.5f), 1.0f));

    // ── Filter (VCF) ───────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "filterCutoff", "Filter Cutoff (SWEEP)",
        juce::NormalisableRange<float>(200.0f, 9000.0f, 1.0f, 0.4f), 3500.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "filterResonance", "Filter Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "filterDecay", "Filter Decay",
        juce::NormalisableRange<float>(0.05f, 5.0f, 0.01f, 0.4f), 0.8f));

    // ── MIDI target ────────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "midiTarget", "MIDI Target",
        juce::StringArray{ "VCO", "Sample", "Both" }, 0));

    return layout;
}

//==============================================================================
DubSirenProcessor::DubSirenProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters_(*this, nullptr, "DubSiren", createParameterLayout())
{}

DubSirenProcessor::~DubSirenProcessor() {}

//==============================================================================
const juce::String DubSirenProcessor::getName() const { return JucePlugin_Name; }
bool DubSirenProcessor::acceptsMidi()  const { return true;  }
bool DubSirenProcessor::producesMidi() const { return false; }
bool DubSirenProcessor::isMidiEffect() const { return false; }
double DubSirenProcessor::getTailLengthSeconds() const { return 2.0; }

int  DubSirenProcessor::getNumPrograms()            { return 1; }
int  DubSirenProcessor::getCurrentProgram()         { return 0; }
void DubSirenProcessor::setCurrentProgram(int)      {}
const juce::String DubSirenProcessor::getProgramName(int) { return {}; }
void DubSirenProcessor::changeProgramName(int, const juce::String&) {}

//==============================================================================
bool DubSirenProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto& outs = layouts.getMainOutputChannelSet();
    return outs == juce::AudioChannelSet::mono()
        || outs == juce::AudioChannelSet::stereo();
}

//==============================================================================
void DubSirenProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    dubOscillator_.Init (static_cast<float>(sampleRate));
    lfo1_.Init          (static_cast<float>(sampleRate));
    lfo2_.Init          (static_cast<float>(sampleRate));
    dubDelay_.Init      (static_cast<float>(sampleRate), 2.0f);
    envelope_.Init      (static_cast<float>(sampleRate));

    // Seed the slewedVCOFreq so portamento doesn't slide from 0 on first note
    slewedVCOFreq_ = *parameters_.getRawParameterValue("vcoRate");

    // Decode all embedded WAV samples (done once at prepare time, not audio thread)
    {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        const char* data[NUM_SAMPLES] = {
            BinaryData::ds01_wav, BinaryData::ds02_wav, BinaryData::ds03_wav,
            BinaryData::ds04_wav, BinaryData::ds05_wav, BinaryData::ds06_wav,
            BinaryData::ds07_wav, BinaryData::ds08_wav, BinaryData::ds09_wav,
            BinaryData::ds10_wav, BinaryData::ds11_wav, BinaryData::ds12_wav,
            BinaryData::ds13_wav, BinaryData::ds14_wav
        };
        const int sizes[NUM_SAMPLES] = {
            BinaryData::ds01_wavSize, BinaryData::ds02_wavSize, BinaryData::ds03_wavSize,
            BinaryData::ds04_wavSize, BinaryData::ds05_wavSize, BinaryData::ds06_wavSize,
            BinaryData::ds07_wavSize, BinaryData::ds08_wavSize, BinaryData::ds09_wavSize,
            BinaryData::ds10_wavSize, BinaryData::ds11_wavSize, BinaryData::ds12_wavSize,
            BinaryData::ds13_wavSize, BinaryData::ds14_wavSize
        };

        for (int i = 0; i < NUM_SAMPLES; ++i)
        {
            auto* stream = new juce::MemoryInputStream(data[i], (size_t)sizes[i], false);
            std::unique_ptr<juce::AudioFormatReader> reader(
                formatManager.createReaderFor(std::unique_ptr<juce::InputStream>(stream)));

            if (reader != nullptr)
            {
                sampleSourceRates_[i] = reader->sampleRate;
                sampleBuffers_[i].setSize((int)reader->numChannels,
                                          (int)reader->lengthInSamples);
                reader->read(&sampleBuffers_[i], 0, (int)reader->lengthInSamples, 0, true, true);
            }
        }

        // Point player at current selection
        auto& buf = sampleBuffers_[currentSampleIdx_];
        if (buf.getNumSamples() > 0)
            samplePlayer_.Load(buf.getReadPointer(0), (size_t)buf.getNumSamples());
    }
}

void DubSirenProcessor::releaseResources() {}

void DubSirenProcessor::triggerButtonOn()     { buttonNoteOnPending_.store(true);  }
void DubSirenProcessor::triggerButtonOff()    { buttonNoteOffPending_.store(true); }
void DubSirenProcessor::sampleButtonTrigger() { sampleTriggerPending_.store(true); }
void DubSirenProcessor::selectSample(int idx) { sampleSelectPending_.store(idx);   }

//==============================================================================
void DubSirenProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                     juce::MidiBuffer&         midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const int   numSamples = buffer.getNumSamples();
    const float sampleRate = static_cast<float>(getSampleRate());

    // ── Cache ALL parameter values at block boundary (never read per-sample) ──
    const float baseVCORate      = *parameters_.getRawParameterValue("vcoRate");
    const float vcoLevel         = *parameters_.getRawParameterValue("vcoLevel");
    const int   vcoWaveformIdx   = static_cast<int>(*parameters_.getRawParameterValue("vcoWaveform"));
    const float portamento       = *parameters_.getRawParameterValue("portamento");

    const float baseDelayTime    = *parameters_.getRawParameterValue("delayTime");
    const float baseDelayFeedback= *parameters_.getRawParameterValue("delayFeedback");
    const float baseDelayWetDry  = *parameters_.getRawParameterValue("delayWetDry");

    const float baseLFO1Rate     = *parameters_.getRawParameterValue("lfo1Rate");
    const float lfo1Amount       = *parameters_.getRawParameterValue("lfo1Amount");
    const int   lfo1TargetIdx    = static_cast<int>(*parameters_.getRawParameterValue("lfo1Target"));
    const int   lfo1WaveformIdx  = static_cast<int>(*parameters_.getRawParameterValue("lfo1Waveform"));

    const float baseLFO2Rate     = *parameters_.getRawParameterValue("lfo2Rate");
    const float lfo2Amount       = *parameters_.getRawParameterValue("lfo2Amount");
    const int   lfo2TargetIdx    = static_cast<int>(*parameters_.getRawParameterValue("lfo2Target"));
    const int   lfo2WaveformIdx  = static_cast<int>(*parameters_.getRawParameterValue("lfo2Waveform"));

    const float baseSamplePlayRate = *parameters_.getRawParameterValue("samplePlayRate");
    const float sampleLevel        = *parameters_.getRawParameterValue("sampleLevel");
    const float filterSweep        = *parameters_.getRawParameterValue("filterCutoff");
    const float filterResonance    = *parameters_.getRawParameterValue("filterResonance");
    const float filterDecay        = *parameters_.getRawParameterValue("filterDecay");
    const int   midiTargetIdx      = static_cast<int>(*parameters_.getRawParameterValue("midiTarget"));
    // 0=VCO, 1=Sample, 2=Both
    const bool  midiToVCO    = (midiTargetIdx == 0 || midiTargetIdx == 2);
    const bool  midiToSample = (midiTargetIdx == 1 || midiTargetIdx == 2);

    // ── Apply waveform / rate selections (block-rate, not per-sample) ────────
    dubOscillator_.SetWaveform(static_cast<DubSiren::DSP::DubOscillator::Waveform>(vcoWaveformIdx));
    dubOscillator_.SetLevel(vcoLevel);

    lfo1_.SetWaveform(static_cast<DubSiren::DSP::LFO::Waveform>(lfo1WaveformIdx));
    lfo1_.SetRate(baseLFO1Rate);

    lfo2_.SetWaveform(static_cast<DubSiren::DSP::LFO::Waveform>(lfo2WaveformIdx));
    lfo2_.SetRate(baseLFO2Rate);

    // Apply base sample playback rate, compensating for source vs host sample rate
    const float srcRate  = static_cast<float>(sampleSourceRates_[currentSampleIdx_]);
    const float rateComp = (srcRate > 0.0f) ? srcRate / static_cast<float>(getSampleRate()) : 1.0f;
    samplePlayer_.SetPlaybackRate(baseSamplePlayRate * rateComp);

    // Portamento coefficient — 1-pole slew on VCO frequency
    const float portaCoeff = (portamento < 0.001f) ? 0.0f
        : std::exp(-1.0f / (portamento * 2.0f * sampleRate));

    // Filter decay coefficient — cutoff slews toward 200Hz after note-off
    const float filterDecayCoeff = std::exp(-1.0f / (filterDecay * sampleRate));
    const float svfQ = juce::jlimit(0.1f, 2.0f,
        2.0f * (1.0f - filterResonance * 0.92f));

    // Decay cutoff toward 200Hz each block (smooth envelope release)
    if (filterDecaying_)
        currentFilterCutoff_ = filterDecayCoeff * currentFilterCutoff_
                              + (1.0f - filterDecayCoeff) * 200.0f;
    // svfF computed per-sample inside loop (allows LFO modulation)

    // ── Button trigger (UI thread → audio thread via atomics) ────────────────
    if (buttonNoteOnPending_.exchange(false))
    {
        currentMidiNote_      = -1;  // use VCO rate knob, not MIDI pitch
        isNoteOn_             = true;
        currentNoteVelocity_  = 1.0f;
        dubOscillator_.SetLevel(vcoLevel);
        envelope_.NoteOn();
    }
    if (buttonNoteOffPending_.exchange(false))
    {
        isNoteOn_        = false;
        currentMidiNote_ = -1;
        envelope_.NoteOff();
    }

    // Sample selection change (swap buffer pointer at block boundary)
    {
        const int sel = sampleSelectPending_.exchange(-1);
        if (sel >= 0 && sel < NUM_SAMPLES)
        {
            currentSampleIdx_ = sel;
            auto& buf = sampleBuffers_[currentSampleIdx_];
            if (buf.getNumSamples() > 0)
                samplePlayer_.Load(buf.getReadPointer(0), (size_t)buf.getNumSamples());
        }
    }

    if (sampleTriggerPending_.exchange(false))
    {
        // One-shot: always retrigger from the start
        sampleLooping_.store(false);
        samplePlayer_.SetLoop(false);
        samplePlayer_.Trigger();
    }

    // ── MIDI iterator ────────────────────────────────────────────────────────
    juce::MidiBuffer::Iterator it(midiMessages);
    juce::MidiMessage msg;
    int samplePos = 0;
    bool hasMsg   = it.getNextEvent(msg, samplePos);

    float* outL = buffer.getWritePointer(0);
    const bool isStereo = (getTotalNumOutputChannels() > 1);

    // ── Sample loop ──────────────────────────────────────────────────────────
    for (int i = 0; i < numSamples; ++i)
    {
        // Process MIDI events at this sample position
        while (hasMsg && samplePos == i)
        {
            if (msg.isNoteOn())
            {
                currentMidiNote_     = msg.getNoteNumber();
                currentNoteVelocity_ = msg.getFloatVelocity();
                if (midiToVCO)
                {
                    isNoteOn_ = true;
                    dubOscillator_.SetLevel(vcoLevel * currentNoteVelocity_);
                    envelope_.NoteOn();
                    // Snap filter to full SWEEP cutoff on note-on
                    currentFilterCutoff_ = filterSweep;
                    filterDecaying_      = false;
                }
                if (midiToSample)
                {
                    // One-shot: each note-on retriggers sample from start
                    samplePlayer_.SetLoop(false);
                    samplePlayer_.Trigger();
                }
            }
            else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
            {
                if (msg.getNoteNumber() == currentMidiNote_)
                {
                    if (midiToVCO)
                    {
                        isNoteOn_        = false;
                        currentMidiNote_ = -1;
                        envelope_.NoteOff();
                        filterDecaying_  = true;  // begin filter sweep decay
                    }
                }
            }
            hasMsg = it.getNextEvent(msg, samplePos);
        }

        // ── LFO 2 — tick first so it can modulate LFO1 before LFO1 runs ──
        const float lfo2Value = lfo2_.ProcessSample();
        const float lfo2Mod   = lfo2Value * lfo2Amount;

        // Mutable working values — LFO2 may adjust before LFO1 reads them
        float currentLFO1Rate    = baseLFO1Rate;
        float currentLFO1Amount  = lfo1Amount;
        float effectiveDelayTime     = baseDelayTime;
        float effectiveDelayFeedback = baseDelayFeedback;
        float effectiveDelayWetDry   = baseDelayWetDry;
        float effectiveSampleRate    = baseSamplePlayRate;
        float effectiveFilterCutoff  = currentFilterCutoff_;

        switch (static_cast<LFO2Target>(lfo2TargetIdx))
        {
            case LFO2Target::LFO1Rate:
                currentLFO1Rate = juce::jlimit(0.05f, 12.0f,
                    baseLFO1Rate * (1.0f + lfo2Mod * 3.0f));
                lfo1_.SetRate(currentLFO1Rate);
                break;
            case LFO2Target::LFO1Amount:
                // Full depth swing — can drive LFO1 from 0 to 2× its set depth
                currentLFO1Amount = juce::jlimit(0.0f, 1.0f,
                    lfo1Amount * (1.0f + lfo2Mod));
                break;
            case LFO2Target::Delay:
                effectiveDelayTime     = juce::jlimit(0.05f, 2.0f,
                    baseDelayTime * (1.0f + lfo2Mod * 0.25f));
                effectiveDelayFeedback = juce::jlimit(0.0f, 0.99f,
                    baseDelayFeedback + lfo2Mod * 0.15f);
                break;
            case LFO2Target::SampleRate:
                effectiveSampleRate = juce::jlimit(0.05f, 4.0f,
                    baseSamplePlayRate + lfo2Mod * 1.5f);
                break;
            default:
                break;
        }

        // ── LFO 1 — runs after LFO2 so LFO2→LFO1rate/amount is applied ──
        const float lfo1Value = lfo1_.ProcessSample();
        const float lfo1Mod   = lfo1Value * currentLFO1Amount;

        // Base VCO frequency — MIDI note if held, else front-panel SIREN knob
        float targetVCORate = (currentMidiNote_ >= 0)
            ? DubSiren::DSP::MidiNoteToFrequency(currentMidiNote_)
            : baseVCORate;

        switch (static_cast<LFO1Target>(lfo1TargetIdx))
        {
            case LFO1Target::VCORate:
                targetVCORate = juce::jlimit(20.0f, 2000.0f,
                    targetVCORate * (1.0f + lfo1Mod * 4.0f));
                break;
            case LFO1Target::Delay:
                effectiveDelayTime     = juce::jlimit(0.05f, 2.0f,
                    effectiveDelayTime * (1.0f + lfo1Mod * 0.25f));
                effectiveDelayFeedback = juce::jlimit(0.0f, 0.99f,
                    effectiveDelayFeedback + lfo1Mod * 0.15f);
                break;
            case LFO1Target::Filter:
                effectiveFilterCutoff = juce::jlimit(200.0f, 9000.0f,
                    currentFilterCutoff_ * (1.0f + lfo1Mod * 0.8f));
                break;
            case LFO1Target::SampleRate:
                effectiveSampleRate = juce::jlimit(0.05f, 4.0f,
                    effectiveSampleRate + lfo1Mod * 1.5f);
                break;
            default:
                break;
        }

        // ── Apply modulated values ────────────────────────────────────────
        samplePlayer_.SetPlaybackRate(effectiveSampleRate);

        // Portamento slew on VCO frequency
        slewedVCOFreq_ = slewedVCOFreq_ * portaCoeff
                       + targetVCORate  * (1.0f - portaCoeff);
        dubOscillator_.SetFrequency(slewedVCOFreq_);

        dubDelay_.SetDelayTime(effectiveDelayTime);
        dubDelay_.SetFeedback(effectiveDelayFeedback);
        dubDelay_.SetWetDry(effectiveDelayWetDry);

        // ── Oscillator + envelope ─────────────────────────────────────────
        const float envVal = envelope_.ProcessSample();
        float osc = (envVal > 0.0f) ? dubOscillator_.ProcessSample() * envVal : 0.0f;

        // ── Sample player ─────────────────────────────────────────────────
        const float smp = samplePlayer_.ProcessSample() * sampleLevel;

        // ── State-variable LP filter — covers VCO + sample (Chamberlin SVF)
        const float preFilt = osc + smp;
        const float svfF = 2.0f * std::sin(
            juce::MathConstants<float>::pi * effectiveFilterCutoff / sampleRate);
        filterLow_  += svfF * filterBand_;
        const float svfHigh = preFilt - filterLow_ - svfQ * filterBand_;
        filterBand_ += svfF * svfHigh;

        outL[i] = filterLow_;
    }

    // ── Apply delay to the mono mix ───────────────────────────────────────────
    dubDelay_.Process(outL, static_cast<size_t>(numSamples));

    // ── Copy to right channel for stereo output ───────────────────────────────
    if (isStereo)
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
}

//==============================================================================
bool DubSirenProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* DubSirenProcessor::createEditor()
{
    return new DubSirenEditor(*this);
}

//==============================================================================
void DubSirenProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DubSirenProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(parameters_.state.getType()))
        parameters_.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DubSirenProcessor();
}
