# RTL·SDR Radio Receiver — VST3 Plugin

> *Tune the airwaves. Sample the ether. Capture what nobody else is hearing.*

A professional VST3 plugin that bridges RTL-SDR USB radio dongles directly into your DAW. Designed for experimental producers, sound designers, and anyone who wants to harvest the raw beauty of the electromagnetic spectrum as an audio source.

The UI is inspired by classic ham radio transceivers — Kenwood TS-830S, Heathkit SB-102, Collins KWM-2A — with a warm amber phosphor VFO display and bakelite-style controls.

---

## Why it sounds interesting

Radio signals carry textures you can't synthesise:
- AM stations bleeding into each other at night (ionospheric skip propagation)
- Aviation VHF with clean, clipped speech and instrument tones
- NOAA Weather Radio — always-on, eerie, atmospheric drone
- Shortwave — distant voices, data bursts, numbers stations
- Ham radio sidebands — unprocessed human voice in USB/LSB
- The noise floor itself — shaped cosmic and thermal noise

Run it through reverb → instant found-sound ambience.
Pitch-shift it → textures nobody has in a sample pack.

---

## Features

### Signal Capture
- **Wideband FM** — broadcast radio, 88–108 MHz and beyond
- **Narrowband FM** — aviation, repeaters, PMR  
- **AM** — shortwave, MW broadcast, aviation ATC
- **USB / LSB** — amateur radio, digital modes, utility stations
- **24 MHz – 1.76 GHz** tuning range (hardware-dependent)

### New in v3.0 — DAW-Friendly Controls

| Control | What it does |
|---------|-------------|
| **AGC** | Automatic gain control — levels out fading signals; 5 ms attack / 150 ms release |
| **Squelch** | Noise gate — silence the carrier hiss between transmissions |
| **Fine Tune ±25 kHz** | Dial in exactly; essential for USB/LSB voice and digital modes |
| **Tape Saturation** | Soft tanh waveshaper — warmth and lo-fi character |

### Visualisations
- **Spectrum Analyzer** — 1024-point FFT with phosphor amber trace; **click any bin to retune instantly**
- **Waterfall Display** — 200-line scrolling history in phosphor green; spot DX, scan for signals visually
- **S-Meter** — vintage needle-sweep signal strength display (S0–S9, +10/+20/+30 dBm)

### Band Presets
One-click tuning: **FM** (98.7 MHz) · **AIR** (121.5 MHz) · **WX** (162.55 MHz) · **HAM** (144.2 MHz) · **HF** (14.3 MHz)

### Preset & History System
- Save / load named frequency presets (XML-persistent)
- Frequency history list (last 20 tuned frequencies)
- 17+ factory presets: FM stations, aviation, NOAA, amateur, shortwave

---

## Signal Chain

```
RTL-SDR Hardware (USB)
    ↓ IQ samples @ 1.024 MSPS (uint8)
SDR Thread → lock-free ring buffer (no blocking)
    ↓
Audio Thread (zero heap allocation in processBlock)
    ├─ FM demod   (polar discriminator + 75 μs de-emphasis)
    ├─ AM demod   (envelope detection + DC blocking)
    └─ SSB demod  (Hilbert transform, USB/LSB)
    ↓
AGC (asymmetric envelope follower, 5 ms attack / 150 ms release)
    ↓
Audio Resampler (Lagrange interpolation, 1.024 MHz → host rate)
    ↓
Tape Saturation (unity-gain normalised tanh waveshaper)
    ↓
Squelch Gate (50 ms RMS envelope tracker, exponential taper)
    ↓
Volume → DAW stereo output
```

---

## Building

### Requirements
- JUCE 7+ (git submodule)
- CMake 3.15+
- C++17 compiler (MSVC 2019+, Clang 10+, GCC 9+)
- librtlsdr (optional — builds in simulation mode without it)

### Quick Start

```bash
# Clone with JUCE submodule
git clone --recurse-submodules https://github.com/maxpatzelt/rtl-sdr-vst3

# Simulation mode (no hardware required)
cmake -B build -DBUILD_WITH_RTLSDR=OFF
cmake --build build --config Release

# With RTL-SDR hardware
cmake -B build -DBUILD_WITH_RTLSDR=ON
cmake --build build --config Release
```

### Install RTL-SDR Driver

**Windows:** Download librtlsdr from [osmocom/rtl-sdr](https://github.com/osmocom/rtl-sdr/releases), run Zadig → WinUSB for your dongle.

**macOS:** `brew install librtlsdr`

**Linux:** `sudo apt install librtlsdr-dev`

---

## Usage in a DAW

1. Load *RTL-SDR Radio* on a Generator / Instrument track
2. Plug in your RTL-SDR dongle (e.g. RTL-SDR Blog V4)
3. Select device from dropdown → **▶ ON AIR**
4. Use **Band** buttons for quick navigation, or **click the spectrum** to retune
5. Set **Squelch** just above noise floor to gate silence
6. Enable **AGC** for consistent levels on fading shortwave
7. Dial in **Saturation** for warmth before recording

### Creative Ideas
- Record NOAA weather radio → slice into ambient pads
- Sample aviation VHF during a storm (lightning impulse noise)
- Feed into a granular synth while sweeping frequency
- Use as convolution reverb impulse source
- Automate the frequency parameter for Doppler-style effects

---

## Technical Notes

### Audio Thread Safety
All processing buffers are pre-allocated in `prepareToPlay()`. `processBlock()` performs **zero heap allocations** on the critical path.

### AGC
Asymmetric single-pole envelope follower at the SDR sample rate (1.024 MHz). Attack 5 ms (fast — catches sudden strong signals), release 150 ms (slow — no pumping on voice). Target level 0.5 FS.

### Squelch
Exponential RMS envelope tracker with 50 ms time constant at host rate. Threshold maps 0–1 → squared RMS for natural taper. Gates output and drains audio buffer while squelched.

### Tape Saturation
`y = tanh(x·drive) / tanh(drive)` where drive = 1 + amount × 8. Unity-gain normalised at all settings. Soft knee, no hard clipping artifacts.

### Denormal Prevention
All recursive filter state variables include `+= 1e-25f` DC kick.

---

## Open Source Inspiration

Architectural patterns and DSP algorithms drawn from:
- [librtlsdr](https://github.com/osmocom/rtl-sdr) — device control
- [rtl_fm / rtl_fm_player](https://github.com/AlbrechtL/rtl_fm_player) — FM demodulation pipeline  
- [SDRPlusPlus](https://github.com/AlexandreRouma/SDRPlusPlus) — modular DSP, ring buffers

No code copied verbatim — architecture and algorithms only.

---

## Roadmap

- [ ] Stereo FM decoding (L/R pilot tone separation)
- [ ] RDS decoder (FM station name / PS / RT)
- [ ] RTL_TCP remote SDR support
- [ ] IQ file record / playback
- [ ] MIDI learn for all parameters
- [ ] HackRF / AirSpy hardware support
- [ ] macOS Universal Binary + AU format
- [ ] CW / digital mode decoder (PSK31, FT8 display)
- [ ] Scanning mode with squelch stop

---

## License

GPL v2 (required by librtlsdr).  
JUCE: GPL v3 / Commercial — [juce.com/juce-7-licence](https://juce.com/juce-7-licence/)

---

**Author:** Max Patzelt · [github.com/maxpatzelt](https://github.com/maxpatzelt/)  
**Brand:** AudioRF  
**Disclaimer:** Receive-only. Use in compliance with local radio regulations.
