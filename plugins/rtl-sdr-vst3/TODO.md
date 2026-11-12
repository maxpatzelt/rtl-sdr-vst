# RTL-SDR VST3 — Development Roadmap

## ✅ Completed

### Phase 1: Core Visualizations (v2.0.0)
- [x] FFT spectrum analyzer (1024-pt, Hann window, 30 FPS)
- [x] Waterfall display (200-line scrolling, color-mapped)
- [x] Signal meter (S0–S9+ dBm)
- [x] Preset manager (17+ factory presets, XML save/load)
- [x] Click-to-tune in spectrum

### Phase 2: Demodulation (v2.1.0)
- [x] AM demodulation (envelope detection + DC blocking)
- [x] SSB demodulation (USB/LSB, Hilbert transform)
- [x] Frequency history (last 20 tuned frequencies)

### Phase 3: Retro UI + DAW Features (v3.0.0)
- [x] Complete retro ham radio visual overhaul
  - [x] Bakelite rotary knobs (custom LookAndFeel)
  - [x] 7-segment amber VFO display
  - [x] Vintage needle S-meter
  - [x] Phosphor amber spectrum analyzer
  - [x] Phosphor green waterfall
  - [x] Dark olive chassis + steel hardware
- [x] AGC — asymmetric envelope follower
- [x] Squelch — RMS envelope gate
- [x] Tape saturation — tanh waveshaper
- [x] Fine tune ±25 kHz
- [x] Band preset buttons (FM / AIR / WX / HAM / HF)
- [x] Audio thread safety — zero heap alloc in processBlock

---

## 🚧 Next Up (Phase 4)

### Stereo FM
- [ ] Pilot tone detection (19 kHz)
- [ ] L+R / L-R subcarrier decoding
- [ ] Stereo DAW output

### RDS Decoder
- [ ] 57 kHz RDS subcarrier extraction
- [ ] PS (station name) + RT (radio text) display in VFO area

### RTL_TCP Remote Support
- [ ] Connect to rtl_tcp server over network
- [ ] Allows use without local hardware / on remote machine

### IQ File Playback
- [ ] Load .iq / .raw files and play back through the demod chain
- [ ] Enables offline processing and demo mode

---

## 📋 Backlog

### MIDI
- [ ] MIDI learn for frequency, gain, volume, mode
- [ ] CC control for scanning
- [ ] MIDI notes → frequency (VCO-style tuning)

### Hardware
- [ ] HackRF One support (libhackrf)
- [ ] AirSpy support (libairspy)
- [ ] RTL-SDR Blog V4 direct sampling (HF without upconverter)
- [ ] Bias-T control (power LNA from USB)

### Cross-Platform
- [ ] macOS Universal Binary (arm64 + x86_64)
- [ ] AU plugin format
- [ ] Linux AppImage
- [ ] Code signing (Windows + macOS)

### Digital Modes
- [ ] CW (Morse) decoder + display
- [ ] PSK31 / RTTY decoder
- [ ] FT8 decode overlay on waterfall

### UX Polish
- [ ] Resizable plugin window
- [ ] Dark/light theme toggle
- [ ] Per-band squelch memory
- [ ] Frequency annotation layer on waterfall
- [ ] Screenshot / export waterfall image

### Performance
- [ ] SIMD IQ conversion (AVX2)
- [ ] Polyphase resampler (better quality than Lagrange)
- [ ] GPU FFT option (CUDA / Metal)
- [ ] Profile demodulator CPU usage

---

## 🐛 Known Issues

- SSB demodulator: minor Hilbert transform ringing on sharp transients
- Frequency history: not persisted across DAW sessions (only runtime)
- No graceful USB disconnect recovery (requires restart)
- Windows: ASIO buffer sizes > 2048 may cause underruns at low signal levels
- processBlock demodulators still do internal push_back — capacity pre-reserved
  but not technically lock-free; tracked for future refactor

---

**Current version:** 3.0.0  
**Last updated:** 2026-03-22  
**Maintainer:** Max Patzelt — https://github.com/maxpatzelt/
