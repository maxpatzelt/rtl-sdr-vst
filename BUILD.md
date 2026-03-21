# Build and Development Guide

## Project Structure

```
RTL-SDR-VST/
├── CMakeLists.txt              # Build configuration
├── README.md                   # Main documentation
├── BUILD.md                    # This file
├── JUCE/                       # JUCE framework (submodule)
└── Source/
    ├── PluginProcessor.h/cpp   # VST3 processor
    ├── PluginEditor.h/cpp      # GUI
    ├── RTLSDRDevice.h/cpp      # RTL-SDR interface
    ├── FMDemodulator.h/cpp     # FM demodulation
    ├── AudioResampler.h/cpp    # Sample rate conversion
    └── RingBuffer.h/cpp        # Thread-safe buffer
```

## Development Environment Setup

### Windows with Visual Studio

1. **Install Prerequisites**
   ```powershell
   # Install Visual Studio 2019+ with C++ tools
   # Install CMake: https://cmake.org/download/
   # Install Git: https://git-scm.com/
   ```

2. **Install RTL-SDR**
   - Download from: https://github.com/osmocom/rtl-sdr/releases
   - Extract and note the path
   - Set environment variables:
     ```powershell
     setx RTLSDR_ROOT "C:\path\to\rtl-sdr"
     ```

3. **Clone and Build**
   ```powershell
   git clone <repo-url> RTL-SDR-VST
   cd RTL-SDR-VST
   git submodule add https://github.com/juce-framework/JUCE.git JUCE
   git submodule update --init --recursive

   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   cmake --build . --config Release
   ```

### macOS with Xcode

1. **Install Prerequisites**
   ```bash
   # Install Xcode from App Store
   xcode-select --install

   # Install Homebrew
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

   # Install dependencies
   brew install cmake librtlsdr
   ```

2. **Clone and Build**
   ```bash
   git clone <repo-url> RTL-SDR-VST
   cd RTL-SDR-VST
   git submodule add https://github.com/juce-framework/JUCE.git JUCE
   git submodule update --init --recursive

   mkdir build
   cd build
   cmake .. -G Xcode
   cmake --build . --config Release
   ```

### Linux (Ubuntu 22.04)

1. **Install Prerequisites**
   ```bash
   sudo apt-get update
   sudo apt-get install -y \
       build-essential \
       cmake \
       git \
       libasound2-dev \
       libfreetype6-dev \
       libx11-dev \
       libxinerama-dev \
       libxrandr-dev \
       libxcursor-dev \
       librtlsdr-dev
   ```

2. **Clone and Build**
   ```bash
   git clone <repo-url> RTL-SDR-VST
   cd RTL-SDR-VST
   git submodule add https://github.com/juce-framework/JUCE.git JUCE
   git submodule update --init --recursive

   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)
   ```

## CMake Options

```bash
# Build without RTL-SDR hardware (simulation mode)
cmake .. -DHAS_RTLSDR=0

# Specify RTL-SDR paths manually
cmake .. -DRTLSDR_LIBRARY=/path/to/librtlsdr.a \
         -DRTLSDR_INCLUDE_DIR=/path/to/rtl-sdr/include

# Build with debug symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build for specific plugin format only
cmake .. -DJUCE_BUILD_VST3=ON -DJUCE_BUILD_AU=OFF
```

## Testing Without Hardware

The plugin includes a simulation mode that generates test IQ data when `HAS_RTLSDR=0`:

```bash
cmake .. -DHAS_RTLSDR=0
cmake --build . --config Release
```

This allows:
- GUI development
- Parameter testing
- Audio pipeline verification
- CI/CD integration

## Debugging

### Enable Debug Logging

In `PluginProcessor.cpp`, add:

```cpp
DBG("SDR Sample Rate: " << sdrDevice.getSampleRate());
DBG("Frequency: " << currentFrequency.load());
```

### Common Issues

**Issue**: CMake can't find RTL-SDR
```bash
# Solution: Specify paths explicitly
cmake .. -DRTLSDR_LIBRARY=/usr/local/lib/librtlsdr.a \
         -DRTLSDR_INCLUDE_DIR=/usr/local/include
```

**Issue**: VST3 doesn't load in DAW
- Check plugin format compatibility (64-bit only)
- Verify code signing (macOS)
- Check plugin validation logs

**Issue**: No audio output
- Verify processBlock is being called (add DBG)
- Check buffer sizes and sample rates
- Ensure SDR is streaming (check status)

## Performance Profiling

### Windows (Visual Studio)
1. Build with RelWithDebInfo
2. Use Performance Profiler in VS
3. Focus on audio thread (processBlock)

### macOS (Instruments)
```bash
# Build with symbols
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build .

# Profile with Instruments
instruments -t "Time Profiler" /path/to/DAW.app
```

### Linux (perf)
```bash
# Record
perf record -g ./build/RTLSDRPlugin_artefacts/Release/Standalone/RTLSDRPlugin

# Analyze
perf report
```

## Code Style

This project follows JUCE conventions:

- CamelCase for classes: `RTLSDRDevice`
- camelCase for methods: `setFrequency()`
- Private members with no prefix
- Use JUCE types where appropriate: `juce::String`, `juce::AudioBuffer`

## Adding New Features

### Example: Adding AM Demodulation

1. **Update FMDemodulator.h**
   ```cpp
   enum class Mode { FM_WIDE, FM_NARROW, AM };
   void setMode(Mode mode);
   ```

2. **Implement in FMDemodulator.cpp**
   ```cpp
   void FMDemodulator::demodulateAM(...)
   {
       // Envelope detection
       for (auto& sample : iq)
           audio.push_back(std::abs(sample));
   }
   ```

3. **Wire to UI**
   ```cpp
   // In PluginEditor.cpp
   modeCombo.addItem("AM", 3);
   ```

4. **Test thoroughly** with real signals

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]

    steps:
    - uses: actions/checkout@v2
      with:
        submodules: recursive

    - name: Install RTL-SDR (Ubuntu)
      if: runner.os == 'Linux'
      run: sudo apt-get install -y librtlsdr-dev

    - name: Install RTL-SDR (macOS)
      if: runner.os == 'macOS'
      run: brew install librtlsdr

    - name: Build
      run: |
        mkdir build && cd build
        cmake .. -DCMAKE_BUILD_TYPE=Release
        cmake --build . --config Release
```

## Release Checklist

- [ ] Version bump in CMakeLists.txt
- [ ] Update README.md with changes
- [ ] Test on all platforms
- [ ] Verify with real RTL-SDR hardware
- [ ] Test in Ableton Live (Windows + macOS)
- [ ] Code sign (macOS)
- [ ] Create installer/package
- [ ] Tag release in git
- [ ] Build release artifacts
- [ ] Update documentation

## Resources

- JUCE Forum: https://forum.juce.com/
- JUCE Docs: https://docs.juce.com/
- RTL-SDR Guide: https://www.rtl-sdr.com/
- DSP Guide: https://www.dspguide.com/

## Contact

For development questions, open an issue on GitHub.
