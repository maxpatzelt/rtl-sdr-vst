# Changelog

All notable changes to the RTL-SDR VST3 Plugin will be documented in this file.

## [2.0.0] - 2026-01-24

### Added - Professional Visualization Suite
- **Spectrum Analyzer**: Real-time FFT analysis with 1024-point FFT
  - Hann windowing for reduced spectral leakage
  - Smoothing factor 0.8 for stable display
  - Green gradient visualization with dB scale (-100 to 0 dB)
  - Frequency labels and grid overlay
  - 30 FPS refresh rate

- **Waterfall Display**: Time-domain spectrum visualization
  - 200-line scrolling history buffer
  - Color-mapped signal intensity (blue → green → yellow → red)
  - HSV color space for vibrant display
  - Synchronized frequency labels
  - 20 FPS refresh rate

- **Signal Strength Meter (S-Meter)**: Professional signal measurement
  - RMS power calculation from IQ samples
  - S-unit scale (S0-S9 + dB over S9)
  - dBm readout (-130 to -40 dBm range)
  - Color-coded meter bar (red/orange/yellow/green)
  - Real-time smoothing for stable readings

- **Preset Manager**: Station management system
  - Save/load favorite frequencies
  - 17+ default presets:
    - FM radio (4 stations)
    - Aviation frequencies (4 channels)
    - NOAA weather radio (4 channels)
    - Amateur radio (3 frequencies)
    - Shortwave time stations (3 WWV frequencies)
  - XML persistence for custom presets
  - One-click preset loading

### Changed - UI Redesign
- Window size increased from 600x500 to 800x700
- Modern dark theme (#1a1a1a background)
- Three-panel layout:
  - Top: Spectrum analyzer (120px)
  - Middle: Waterfall display (120px)
  - Bottom: Controls and parameters
- Rounded corner panels for professional studio look
- Improved spacing and alignment

### Technical Improvements
- Exposed IQ buffer for real-time visualization
- Lock-free ring buffer access for visualizers
- Separate timer-based rendering for each visualizer
- Optimized FFT processing with windowing
- Efficient color mapping algorithms

## [1.0.0] - 2026-01-23

### Initial Release
- RTL-SDR hardware integration via librtlsdr
- FM demodulation (wideband/narrowband)
- Audio resampling (1.024 MHz → 48 kHz)
- Basic UI with frequency/gain/mode/volume controls
- Frequency presets (FM, Aviation, Weather)
- VST3 Generator category
- Simulation mode for testing without hardware
- Thread-safe IQ and audio buffers
- Modern JUCE parameter API integration
