# Future Enhancements & TODOs

## High Priority

### AM Demodulation
- [ ] Implement envelope detection algorithm
- [ ] Add DC offset removal
- [ ] Test with aviation band (118-137 MHz)
- [ ] Update UI to show AM mode

### Spectrum Analyzer
- [ ] Add FFT analysis of IQ stream
- [ ] Display real-time spectrum in GUI
- [ ] Add waterfall display option
- [ ] Allow clicking spectrum to tune

### Performance Optimization
- [ ] Profile demodulation code
- [ ] Consider SIMD for IQ conversion
- [ ] Optimize ring buffer performance
- [ ] Reduce memory allocations in audio thread

## Medium Priority

### Additional Demodulation Modes
- [ ] SSB (Single Sideband)
  - USB (Upper Sideband)
  - LSB (Lower Sideband)
- [ ] CW (Morse Code)
- [ ] Digital modes (PSK31, RTTY)

### GUI Enhancements
- [ ] Add signal strength meter (S-meter)
- [ ] Frequency bookmarks/favorites
- [ ] Scan mode (auto-scan frequencies)
- [ ] Band selector (FM, Aviation, Ham, etc.)
- [ ] Dark/Light theme toggle

### Recording Features
- [ ] Record to WAV file
- [ ] Record IQ samples to file
- [ ] Playback IQ files (for testing)
- [ ] Timestamp recordings

### Signal Processing
- [ ] Automatic Gain Control (AGC)
- [ ] Noise reduction/blanker
- [ ] Squelch (silence output below threshold)
- [ ] Stereo FM decoding (L/R separation)
- [ ] RDS/RBDS data decoder

## Low Priority

### Advanced Features
- [ ] Multiple SDR device support
- [ ] Remote SDR via network (RTL_TCP)
- [ ] Plugin chain (pre/post processing)
- [ ] MIDI learn for parameters
- [ ] Preset system (save favorite stations)

### Platform Support
- [ ] AAX format (Pro Tools)
- [ ] AU format (macOS Audio Units)
- [ ] Standalone application mode
- [ ] iOS/Android versions

### Documentation
- [ ] Video tutorials
- [ ] Example projects (Ableton Live sets)
- [ ] Frequency guide by region
- [ ] Legal/compliance guide by country

## Code Quality

### Testing
- [ ] Unit tests for DSP components
- [ ] Integration tests
- [ ] CI/CD pipeline (GitHub Actions)
- [ ] Automated plugin validation
- [ ] Performance benchmarks

### Code Improvements
- [ ] Add more inline documentation
- [ ] Improve error handling
- [ ] Add logging system
- [ ] Code coverage analysis
- [ ] Static analysis (clang-tidy)

## Known Issues

### To Fix
- [ ] Thread safety audit (ensure no race conditions)
- [ ] Memory leak check (Valgrind/ASAN)
- [ ] Handle USB disconnect gracefully
- [ ] Improve startup latency
- [ ] Fix potential buffer overflows

### To Investigate
- [ ] Occasional crackling on weak signals
- [ ] CPU spikes when changing frequency
- [ ] GUI responsiveness with multiple instances
- [ ] Sample rate mismatch edge cases

## Community Requests

### Feature Requests (Placeholder)
- [ ] User request #1: ...
- [ ] User request #2: ...

## Research & Exploration

### Experimental Ideas
- [ ] Machine learning for signal classification
- [ ] Automatic station identification
- [ ] Real-time translation of weather broadcasts
- [ ] Integration with online SDR databases
- [ ] Collaborative listening (multiple users)

## Build System

### Improvements
- [ ] CPack for installer generation
- [ ] Codesigning automation (macOS/Windows)
- [ ] Debian package creation
- [ ] Homebrew formula
- [ ] Chocolatey package (Windows)

## Dependencies

### Updates
- [ ] Keep JUCE updated
- [ ] Monitor librtlsdr updates
- [ ] Check for security updates
- [ ] Test with latest DAW versions

---

## How to Contribute

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
