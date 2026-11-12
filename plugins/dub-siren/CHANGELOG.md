# Changelog — Dub Siren

## v2.0.0 — 2026-03-23

### DSP — DubOscillator
- **PolyBLEP anti-aliasing** on Square and Saw waveforms — eliminates aliasing staircase on high-pitched notes
- Added **Sawtooth** and **Triangle** waveforms alongside Square
- Replaced `rand()` (non-deterministic, thread-unsafe) with **xorshift32 PRNG** — fully deterministic noise for transistor grit character
- Analog drift oscillator now uses **sample-rate-aware increment** (`2.7 Hz / sampleRate`) instead of hardcoded `0.0001f`

### DSP — LFO
- Added **4 waveforms:** Sine, Triangle, Square, Sample-and-Hold
- S+H uses internal xorshift32 PRNG — new value sampled at each phase wrap (clock rising edge)
- `SetWaveform()` API added; previous sine-only behaviour is the default

### PluginProcessor
- **Zero per-sample parameter polling** — all `APVTS::getRawParameterValue()->load()` calls moved to block boundary, cached as `const float`. Sample loop uses only the cached copies.
- **New parameters:** `vcoWaveform`, `portamento`, `lfo1Waveform`, `lfo2Waveform`
- **Portamento / glide** — 1-pole slew limiter on VCO frequency (0–2 s, `portaCoeff = exp(-1 / portamento·2·sampleRate)`)
- **Stereo output** — `isBusesLayoutSupported` now accepts stereo; mono is copied to right channel in `processBlock`
- LFO2 now ticks _before_ LFO1 in the sample loop so LFO2→LFO1Rate modulation is applied to LFO1 before it processes
- Removed redundant `updateDSPFromParameters()` call at block start (replaced by inline cached reads)
- Waveform and rate selections applied once per block (block-rate, not per-sample)
- Removed `BinaryData` dependency entirely (panel image removed)

### UI — PluginEditor
- **Complete procedural UI rewrite** — no `panel.jpg` or `BinaryData` dependency
- New **DubLookAndFeel** replacing RastaKnobLookAndFeel:
  - Dark charcoal-warm chassis background (`#161210`)
  - Knob body: radial-gradient bakelite with top-left sheen
  - Amber arc indicator + cream pointer line + amber tip dot
  - Centre screw cap with slot aligned to pointer angle
  - Custom ComboBox with dark body + amber arrow
- **All labels positioned below their knobs** (not overlaid on them)
- Controls grouped into 4 section boxes: **VCO**, **DELAY**, **LFO 1**, **LFO 2**
- Added **VCO Waveform** ComboBox (Square / Saw / Tri)
- Added **Portamento** knob
- Added **LFO Waveform** ComboBoxes (Sine / Tri / Square / S+H) for both LFOs
- Window resized to **720 × 490** (from 800 × 600)
- Header bar: "DUB SIREN" in amber monospaced, green status LED, AudioRF brand

### Known issues carried forward
- ADSR sliders not yet exposed in the UI (still hardcoded defaults)
- Preset save/load not yet implemented

---

## v1.0.0 — initial release

- Mono square-wave VCO with analog drift
- Dual sine-wave LFOs with 3-target routing each
- Dub feedback delay up to 2 s with wobble
- ADSR envelope, MIDI note tracking, velocity scaling
- RastaKnobLookAndFeel (red/gold/green cycling knobs)
- panel.jpg background (removed in v2.0)
