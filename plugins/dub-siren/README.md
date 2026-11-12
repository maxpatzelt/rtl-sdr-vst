# Dub Siren

> *The wail of Kingston. Pure analogue dub energy in your DAW.*

A stereo VST3 dub synthesizer — band-limited VCO with polyBLEP anti-aliasing, two multi-waveform LFOs with flexible routing, portamento, and a feedback delay with analog wobble. Inspired by the hardware dub sirens of King Tubby, Lee "Scratch" Perry, and Jah Shaka.

**Author:** Max Patzelt · https://github.com/maxpatzelt/
**Brand:** AudioRF

---

## Why it sounds like dub

Real Jamaican dub gear was built from surplus oscillator boards and echo boxes wired together on improvised consoles. This plugin captures that character:

- Square waves with slight transistor-noise grit and phase drift (not a clean digital sine)
- Delay feedback that pushes into near-infinite repeats with subtle tape wobble
- LFOs that go up to 80 Hz — from slow swell to audio-rate FM tremolo
- Portamento that glides between notes the way a hand-cranked VCO knob would

---

## Features

### VCO
- **3 waveforms:** Square (polyBLEP-antialiased), Saw (polyBLEP), Triangle
- **Portamento / Glide** — 1-pole slew limiter, 0–2 seconds
- Analog drift simulation (sample-rate-aware slow phase modulation)
- Xorshift32 noise for transistor texture — deterministic, no `rand()`
- MIDI note tracking + velocity scaling

### Dual LFO System
- **4 waveforms each:** Sine, Triangle, Square, Sample-and-Hold
- LFO rates 0.1–80 Hz — slow swell to audio-rate FM
- **LFO 1 targets:** VCO Rate · Delay Time · Delay Feedback
- **LFO 2 targets:** LFO 1 Rate · LFO 1 Amount · Delay Wet/Dry

### Delay
- Up to 2-second delay time
- Feedback up to 0.95 (near-infinite repeats)
- Wet/dry mix
- Subtle tape-wobble instability

### Audio Engine
- Stereo output
- Zero heap allocation in `processBlock` — all buffers pre-allocated
- APVTS parameter reads cached once per block (no per-sample atomic loads)
- Full ADSR amplitude envelope

---

## Parameters

| ID | Name | Range | Default | Notes |
|----|------|--------|---------|-------|
| `vcoRate` | VCO Rate | 20–2000 Hz | 440 Hz | Base frequency (overridden by MIDI) |
| `vcoLevel` | VCO Level | 0–1 | 0.8 | |
| `vcoWaveform` | VCO Waveform | Square / Saw / Tri | Square | PolyBLEP on Square + Saw |
| `portamento` | Portamento | 0–1 | 0 | Maps to 0–2 s slew |
| `delayTime` | Delay Time | 0.001–2 s | 0.375 s | Classic 3/8-note dub echo |
| `delayFeedback` | Delay Feedback | 0–0.95 | 0.6 | |
| `delayWetDry` | Delay Wet/Dry | 0–1 | 0.4 | |
| `lfo1Rate` | LFO 1 Rate | 0.1–80 Hz | 2 Hz | |
| `lfo1Amount` | LFO 1 Amount | 0–1 | 0.5 | |
| `lfo1Target` | LFO 1 Target | None / VCO Rate / Delay Time / Delay FB | None | |
| `lfo1Waveform` | LFO 1 Waveform | Sine / Tri / Square / S+H | Sine | |
| `lfo2Rate` | LFO 2 Rate | 0.1–80 Hz | 0.5 Hz | |
| `lfo2Amount` | LFO 2 Amount | 0–1 | 0.3 | |
| `lfo2Target` | LFO 2 Target | None / LFO1 Rate / LFO1 Amount / Delay W/D | None | |
| `lfo2Waveform` | LFO 2 Waveform | Sine / Tri / Square / S+H | Sine | |

---

## Signal Chain

```
MIDI Note → Portamento Slew → VCO (Square/Saw/Tri, PolyBLEP)
                                 ↑
           LFO 2 → LFO 1 rate / amount
               ↓
           LFO 1 → VCO Rate / Delay Time / Delay Feedback
                                 ↓
                            ADSR Envelope
                                 ↓
                           Dub Delay (2s, wobble)
                                 ↓
                         Stereo DAW Output
```

---

## Building

### Requirements

- JUCE 7+ (git submodule at `libs/JUCE`)
- CMake 3.15+
- C++17 compiler: MSVC 2019+, Clang 10+, or GCC 9+

### Quick Start

```bash
# Clone with JUCE submodule
git clone --recurse-submodules https://github.com/maxpatzelt/dub-siren

# Configure & build
cmake -B build
cmake --build build --config Release
```

The VST3 will be at: `build/DubSiren_artefacts/Release/VST3/Dub Siren.vst3`

### Install

**Windows:**
```powershell
Copy-Item "build\DubSiren_artefacts\Release\VST3\Dub Siren.vst3" `
          "$env:CommonProgramFiles\VST3\" -Recurse -Force
```

**macOS:**
```bash
cp -r "build/DubSiren_artefacts/Release/VST3/Dub Siren.vst3" \
      ~/Library/Audio/Plug-Ins/VST3/
```

---

## DAW Usage Tips

- Set LFO 1 → VCO Rate, dial WOBBLE to taste for the classic siren sweep
- Push REPEATS above 0.85 for infinite dub spirals — automate ECHO TIME while it feeds back
- Set LFO 2 → LFO 1 Rate for a "siren that wobbles in waves"
- Use S+H waveform on LFO 2 for random rhythm-sync glitching
- Portamento + slow LFO on VCO Rate = ship-horn texture when fed into reverb

---

## Technical Notes

### PolyBLEP Anti-Aliasing
Square and Saw waveforms apply a polynomial band-limited step (PolyBLEP) correction
at each transition discontinuity, eliminating the aliasing staircase audible on
naive oscillators at higher pitches.

### Xorshift32 PRNG
All noise and S+H stochastic behaviour uses a per-instance xorshift32 PRNG seeded
at construction. This is deterministic, lock-free, and has no dependency on the
platform `rand()` call.

### Audio Thread Safety
All `AudioProcessorValueTreeState` raw parameter loads happen exactly once per
`processBlock()` call (at the top of the function, before the sample loop). The
sample loop uses only the cached copies.

---

## Roadmap

- [x] PolyBLEP antialiased VCO waveforms (Square, Saw, Triangle)
- [x] Multi-waveform LFOs (Sine, Tri, Square, S+H)
- [x] Portamento / pitch glide
- [x] Stereo output
- [ ] ADSR knobs exposed in UI
- [ ] Preset save/load
- [ ] Second oscillator / octave spread
- [ ] Scanning / auto-sweep mode
- [ ] macOS Universal Binary (Intel + Apple Silicon)
- [ ] AAX format (Pro Tools)

---

## Open Source Inspiration

DSP patterns drawn from:
- [Mutable Instruments Braids](https://github.com/pichenettes/eurorack) — polyBLEP waveforms
- Classic dub hardware: Watkins Copicat, Roland Space Echo RE-201

No code copied verbatim — architecture and algorithms only.

---

## License

MIT

JUCE is licensed under GPL v3 or commercial — see https://juce.com/juce-7-licence/

**Author:** Max Patzelt · [github.com/maxpatzelt](https://github.com/maxpatzelt/)
**Disclaimer:** Receive-only radio content (RTL-SDR companion) is separate.
