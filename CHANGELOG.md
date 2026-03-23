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

## [3.0.0] - 2026-03-22

### Added — Retro Ham Radio UI
- **Complete visual overhaul**: Kenwood/Heathkit/Collins inspired front panel
  - `RetroLookAndFeel` class: bakelite rotary knobs with amber arc indicators,
    cream pointer, centre set-screw detail
  - **7-segment VFO display**: amber phosphor on near-black background,
    8-digit XXXX.XXX MHz format with dim off-segments
  - **Vintage S-Meter**: ivory face, engraved scale, needle-sweep with
    S0–S9 and +10/+20/+30 dBm red zone
  - Dark olive chassis (`#0D0D09`) + steel hardware accents
  - Corner rivets / screw detail in header
  - Engraved divider lines between all panel sections

### Added — Spectrum Analyzer (phosphor amber theme)
- Warm amber trace (`#FFB300`) on near-black phosphor-tube background
- Phosphor green (`#39FF14`) highlight layer on peak signals (> -20 dB)
- Amber fill gradient under curve
- dB scale labels on right edge, frequency labels at bottom
- Dim / major horizontal grid lines

### Added — Waterfall Display (phosphor green theme)
- New colormap: silent → deep charcoal green → bright phosphor green → amber/gold peak
- Complements amber spectrum above; creates visual distinction between the two views
- Sparse vertical grid + amber frequency labels

### Added — New DSP Parameters (v3.0)
- **AGC** (`agcEnabled`, bool): asymmetric envelope follower — 5 ms attack at SDR
  rate (1.024 MHz), 150 ms release; target level 0.5 FS
- **Squelch** (`squelch`, 0–1): 50 ms RMS envelope gate with exponential threshold
  taper; drains audio buffer while squelched
- **Tape Saturation** (`saturation`, 0–1): unity-gain normalised tanh waveshaper
  — `y = tanh(x·drive) / tanh(drive)`, drive up to 9×
- **Fine Tune** (`fineTune`, ±25 kHz): offset applied on top of main frequency,
  updates hardware in real-time; shown in VFO display

### Added — Band Preset Buttons
- One-click: FM (98.7), AIR (121.5), WX (162.55), HAM (144.2), HF (14.3) MHz

### Fixed — Audio Thread Violations
- `tempIQBuffer.resize(toRead)` removed from `processBlock()` — buffer pre-allocated
  to 64k in `prepareToPlay()`
- `std::vector<float> outputSamples(samplesNeeded)` removed — pre-allocated member
- `std::vector<float> resampledAudio` local removed — pre-allocated member
- All vector capacities pre-reserved in `prepareToPlay()` to prevent
  `push_back()` heap allocation in demodulators

### Changed
- Window resized from 800×700 to 860×600
- Frequency slider replaced by rotary GAIN, VOLUME, SQUELCH, FINE, SATURATION knobs
- Mode selector (ComboBox) replaced by illuminated toggle buttons: WFM / NFM / AM / USB / LSB
- `getStateInformation` / `setStateInformation` extended to include all v3.0 parameters
- VFO display re-reads `frequency + fineTune` offset every timer tick

### Technical
- AGC / squelch / saturation run in audio thread with no allocation
- `signalEnvelope` updated from resampled audio, used by squelch gate
- Fine tune offset applied before frequency write to hardware (in MHz)
