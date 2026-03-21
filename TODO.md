# RTL-SDR VST3 Plugin - Development Roadmap

## ✅ Phase 1: Professional Visualizations (COMPLETED v2.0.0)
- [x] Real-time FFT spectrum analyzer (1024-point with smoothing)
- [x] Waterfall display with color mapping (200-line history)
- [x] Signal strength meter (S-meter with dBm/S-units)
- [x] Preset manager with 17+ default stations
- [x] Modern dark theme UI (800x700)
- [x] Build and deploy to GitHub

## ✅ Phase 2: Enhanced Demodulation (COMPLETED v2.1.0)
- [x] AM demodulation — envelope detection, DC offset removal (`AMDemodulator.cpp`)
- [x] SSB demodulation (USB/LSB) — Hilbert transform, fine tuning (`SSBDemodulator.cpp`)
- [x] Narrow FM support for NOAA weather

## 🚧 Phase 2b: Demodulation Polish (NEXT)
- [ ] AGC (Automatic Gain Control) for AM/NFM
- [ ] Fine frequency tuning UI (+/- 10 kHz control)
- [ ] Improved squelch for weather channels
- [ ] Test SSB with amateur radio (14 MHz, 7 MHz)

## 📊 Phase 3: Advanced Visualization

### Spectrogram View
- [ ] Resizable spectrogram display
- [ ] Time/frequency zoom controls
- [ ] Screenshot/export capability
- [ ] Click-to-tune functionality

### Constellation Diagram
- [ ] I/Q scatter plot for signal quality
- [ ] Real-time phase/amplitude visualization
- [ ] Signal quality metrics
- [ ] Useful for diagnosing issues

### Audio Spectrum Analyzer
- [ ] Post-demodulation audio FFT
- [ ] Separate from RF spectrum
- [ ] Shows demodulated audio content
- [ ] Bass/treble visualization

## 🎛️ Phase 4: User Experience

### Frequency Memory
- [ ] Recently tuned frequencies list
- [ ] Quick-access history (last 10)
- [ ] Station identification labels
- [ ] Persistent across sessions

### Scanning Mode
- [ ] Auto-scan frequency ranges
- [ ] Squelch threshold control
- [ ] Stop on active signal detection
- [ ] Scan lists by band

### Recording Features
- [ ] Record demodulated audio to WAV
- [ ] Save IQ data for later analysis
- [ ] Export presets with recordings
- [ ] Timestamp recordings

## 🎹 Phase 5: MIDI Integration
- [ ] Map frequency to MIDI notes/CC
- [ ] MIDI learn for all parameters
- [ ] Control preset switching via MIDI
- [ ] MIDI sync for scanning mode

## 🎨 Phase 6: Audio Effects Chain
- [ ] Built-in reverb/delay effects
- [ ] Pitch shifting
- [ ] Granular synthesis from radio signals
- [ ] Lo-fi/bit-crushing effects
- [ ] Ring modulation

## 🔧 Phase 7: Performance & Polish

### Optimization
- [ ] Profile demodulation code
- [ ] GPU acceleration for FFT
- [ ] Multi-threaded demodulation
- [ ] SIMD for IQ conversion
- [ ] Reduce CPU usage (target <5%)
- [ ] Optimize ring buffer performance

### Hardware Support
- [ ] Support for HackRF One
- [ ] AirSpy devices
- [ ] RTL-SDR Blog v4 features
- [ ] Bias-T control for LNA
- [ ] Multiple simultaneous SDRs

### Cross-Platform
- [ ] macOS build and testing
- [ ] Linux VST3 support
- [ ] ARM64 builds (Apple Silicon)
- [ ] AAX format (Pro Tools)
- [ ] AU format (macOS Audio Units)

## 📚 Phase 8: Documentation & Community

### User Manual
- [ ] Getting started guide
- [ ] Frequency band reference
- [ ] Troubleshooting section
- [ ] Legal/compliance guide by country

### Video Tutorials
- [ ] Basic setup walkthrough
- [ ] Creative music production examples
- [ ] Advanced SDR techniques
- [ ] Example Ableton Live projects

### Community Features
- [ ] Preset sharing platform
- [ ] Regional frequency databases
- [ ] User-contributed station lists
- [ ] Forum/Discord community

## 🎯 Next Immediate Steps

1. **Test v2.0.0 in Ableton**
   - Verify spectrum/waterfall/S-meter rendering
   - Check preset loading functionality
   - Validate UI layout and responsiveness
   - Test with simulation mode

2. **Implement AM Demodulation**
   - Create AMDemodulator.h/cpp
   - Envelope detection algorithm
   - Integrate into mode selector
   - Test with aviation frequencies

3. **Add Click-to-Tune**
   - Spectrum analyzer click detection
   - Convert pixel position to frequency
   - Update frequency parameter

4. **Create Demo Video**
   - Record plugin in action
   - Show all visualization features
   - Upload to GitHub README and YouTube

## 💡 Feature Ideas (Community Wishlist)

- [ ] Direct sampling mode for HF (0-30 MHz)
- [ ] Custom waterfall color schemes (theme editor)
- [ ] Signal recording buffer (loop last 30s)
- [ ] RDS decoder for FM radio station info
- [ ] Trunked radio support (P25, DMR)
- [ ] WebSDR remote tuning integration
- [ ] Stereo FM decoding (L/R separation)
- [ ] CW (Morse Code) decoder with visual display
- [ ] Digital modes (PSK31, RTTY, FT8)
- [ ] Automatic station identification
- [ ] Remote SDR via network (RTL_TCP)

## 🧪 Code Quality & Testing

### Testing
- [ ] Unit tests for DSP components
- [ ] Integration tests for SDR pipeline
- [ ] CI/CD pipeline (GitHub Actions)
- [ ] Automated plugin validation (pluginval)
- [ ] Performance benchmarks

### Code Improvements
- [ ] Add more inline documentation
- [ ] Improve error handling throughout
- [ ] Add comprehensive logging system
- [ ] Code coverage analysis
- [ ] Static analysis (clang-tidy)

## 🐛 Known Issues & Fixes

### To Fix
- [ ] Thread safety audit (race condition check)
- [ ] Memory leak check (Valgrind/ASAN)
- [ ] Handle USB disconnect gracefully
- [ ] Improve startup latency
- [ ] AGC for weak signals

### To Investigate
- [ ] GUI responsiveness with multiple instances
- [ ] Sample rate mismatch edge cases
- [ ] High CPU on frequency changes
- [ ] Waterfall performance on slower systems

## 🏗️ Build System Improvements

- [ ] CPack for installer generation
- [ ] Code signing automation (macOS/Windows)
- [ ] Debian package creation
- [ ] Homebrew formula (macOS)
- [ ] Chocolatey package (Windows)
- [ ] Standalone application mode

## 📦 Dependencies & Maintenance

- [ ] Keep JUCE framework updated
- [ ] Monitor librtlsdr updates
- [ ] Security vulnerability scanning
- [ ] Test with latest DAW versions
- [ ] Update documentation on changes

---

**Current Version**: 2.1.0
**Last Updated**: March 2026
**Maintainer**: Max Patzelt — https://github.com/maxpatzelt/
**Status**: ✅ Phase 1 + Phase 2 complete — Spectrum, Waterfall, S-Meter, Presets, AM, SSB all implemented.

## 📝 Notes

The plugin is now feature-complete for professional SDR visualization! Next priorities are AM demodulation and click-to-tune functionality. The codebase is clean, well-documented, and ready for community contributions.

1. Pick a TODO item
2. Create an issue discussing your approach
3. Fork and create a feature branch
4. Implement with tests
5. Update documentation
6. Submit pull request

## Priority Guide

- **High**: Core functionality, stability, performance
- **Medium**: Nice-to-have features, UX improvements
- **Low**: Advanced features, experimental ideas

---

**Last Updated**: January 2026
**Maintainer**: (Your name here)
