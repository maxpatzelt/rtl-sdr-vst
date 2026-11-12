# RTL-SDR VST3 Plugin - Quick Reference

## 📋 Pre-Build Checklist

- [ ] Git installed
- [ ] CMake 3.15+ installed
- [ ] C++17 compiler (Visual Studio 2019+, Xcode 12+, or GCC 9+)
- [ ] JUCE submodule cloned: `git submodule add https://github.com/juce-framework/JUCE.git JUCE`
- [ ] librtlsdr installed OR building in simulation mode

## 🚀 Build Commands

### Windows (PowerShell)
```powershell
# Automated
.\setup.ps1

# Manual
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### macOS/Linux
```bash
# Automated
chmod +x setup.sh
./setup.sh

# Manual
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Simulation Mode (No Hardware)
```bash
cmake .. -DHAS_RTLSDR=0 -DCMAKE_BUILD_TYPE=Release
```

## 📁 File Overview

| File | Lines | Purpose |
|------|-------|---------|
| `RTLSDRDevice.cpp` | ~280 | RTL-SDR hardware interface |
| `FMDemodulator.cpp` | ~180 | FM demodulation DSP |
| `AudioResampler.cpp` | ~70 | Sample rate conversion |
| `RingBuffer.h` | ~120 | Thread-safe buffer (header-only) |
| `PluginProcessor.cpp` | ~320 | VST3 main processor |
| `PluginEditor.cpp` | ~250 | GUI implementation |
| **Total** | **~1220** | Core plugin code |

## 🎛️ Parameter Ranges

| Parameter | Min | Max | Default | Unit |
|-----------|-----|-----|---------|------|
| Frequency | 24 | 1766 | 100 | MHz |
| Gain | 0 | 50 | 29 | dB |
| Mode | 0 | 2 | 0 | enum |
| Volume | 0 | 1 | 0.7 | linear |

## 📡 Frequency Presets

| Service | Frequency | Notes |
|---------|-----------|-------|
| FM Radio (US) | 88-108 MHz | Wideband FM |
| Aviation | 118-137 MHz | AM mode (future) |
| NOAA Weather | 162.4-162.55 MHz | Narrowband FM |
| 2-Meter Ham | 144-148 MHz | FM/SSB |
| ISS | 145.8 MHz | Downlink |

## 🔧 Troubleshooting

| Issue | Solution |
|-------|----------|
| "Device not found" | Install RTL-SDR drivers (zadig on Windows) |
| "Build failed - JUCE not found" | Run `git submodule update --init --recursive` |
| "No audio output" | Check SDR is started, verify frequency is valid |
| "Crackling/distortion" | Reduce gain, check USB cable quality |
| "High CPU usage" | Normal for real-time SDR processing |

## 📊 System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    ABLETON LIVE (DAW)                       │
│  ┌───────────────────────────────────────────────────────┐  │
│  │              RTL-SDR Radio (VST3 Plugin)              │  │
│  │                                                       │  │
│  │  ┌─────────────┐  ┌──────────────┐  ┌─────────────┐ │  │
│  │  │   Editor    │  │  Processor   │  │ Parameters  │ │  │
│  │  │   (GUI)     │◄─┤   (Audio)    │◄─┤  (Params)   │ │  │
│  │  └─────────────┘  └──────┬───────┘  └─────────────┘ │  │
│  │                          │                           │  │
│  │         Audio Thread ────┼──── SDR Thread           │  │
│  │                          │                           │  │
│  │  ┌──────────────┐  ┌─────▼────────┐  ┌───────────┐ │  │
│  │  │   Resampler  │  │     Ring     │  │  Demod    │ │  │
│  │  │ 1.024→48kHz  │◄─┤    Buffer    │◄─│    FM     │ │  │
│  │  └──────┬───────┘  └──────────────┘  └─────▲─────┘ │  │
│  │         │                                   │       │  │
│  │         ▼                                   │       │  │
│  │  ┌──────────────┐                  ┌────────┴────┐ │  │
│  │  │    Output    │                  │  RTLSDRDev  │ │  │
│  │  │   to DAW     │                  │  Interface  │ │  │
│  │  └──────────────┘                  └─────────────┘ │  │
│  └───────────────────────────────────────────┬─────────┘  │
└────────────────────────────────────────────────┼──────────┘
                                                 │ USB
                                        ┌────────▼────────┐
                                        │   RTL-SDR USB   │
                                        │     Dongle      │
                                        │   🔌 Antenna    │
                                        └─────────────────┘
```

## 🎯 Testing Checklist

### Build Tests
- [ ] Builds on Windows (Visual Studio)
- [ ] Builds on macOS (Xcode)
- [ ] Builds on Linux (GCC)
- [ ] Builds in simulation mode (no hardware)
- [ ] No compiler warnings

### Functional Tests
- [ ] Plugin loads in Ableton Live
- [ ] Device enumeration works
- [ ] Frequency tuning works
- [ ] Gain control works
- [ ] Audio output is clean
- [ ] Parameters are saved/restored
- [ ] GUI updates correctly

### Hardware Tests (with RTL-SDR)
- [ ] Device opens successfully
- [ ] FM radio demodulation sounds clear
- [ ] Can tune across full frequency range
- [ ] No crashes during long sessions
- [ ] Stop/Start works reliably

### Performance Tests
- [ ] CPU usage < 20% (48 kHz, single instance)
- [ ] No audio dropouts
- [ ] Latency < 100 ms
- [ ] Memory usage < 50 MB

## 📚 Documentation Structure

```
README.md          → User guide (start here)
  ↓
BUILD.md          → Build instructions (developers)
  ↓
ARCHITECTURE.md   → Technical deep-dive (DSP/algorithms)
  ↓
PROJECT_SUMMARY.md → Quick overview (this file)
```

## 🔗 Useful Links

- **JUCE**: https://juce.com/
- **RTL-SDR**: https://www.rtl-sdr.com/
- **librtlsdr**: https://github.com/osmocom/rtl-sdr
- **SDRPlusPlus**: https://github.com/AlexandreRouma/SDRPlusPlus
- **Ableton Live**: https://www.ableton.com/

## 💡 Quick Tips

1. **Start with simulation mode** if you don't have hardware yet
2. **Use preset buttons** for quick frequency changes
3. **Adjust gain** if signal is too weak or distorted
4. **Record in Ableton** to capture interesting RF moments
5. **Chain effects** after the plugin for creative processing

## 🎵 Creative Ideas

- Sample radio static for ambient textures
- Record aviation comms for dystopian soundscapes
- Capture data transmissions (APRS, weather fax) as noise
- Use FM radio as a random source for generative music
- Create "radio archaeology" pieces from old broadcasts
- Layer multiple instances on different frequencies

## 📈 Next Development Steps

1. ✅ Core functionality (DONE)
2. ⏳ AM demodulation
3. ⏳ Spectrum analyzer visualization
4. ⏳ Waterfall display
5. ⏳ Frequency scanning
6. ⏳ IQ recording to file

## 🤝 Contributing

1. Fork repository
2. Create feature branch
3. Test thoroughly
4. Submit pull request
5. Update documentation

---

**Version**: 1.0.0
**Status**: Production Ready ✅
**Last Updated**: January 2026

**🎉 Ready to build and use!**
