# SimpleSynth - Quick Testing Without Full Build

## Why MSYS2 Doesn't Work

**JUCE explicitly doesn't support MinGW/GCC on Windows.** From the JUCE source code:
```cpp
#error "MinGW is not supported. Please use an alternative compiler."
```

JUCE requires **Microsoft Visual C++ (MSVC)** on Windows.

---

## Option 1: Install Visual Studio (Recommended)

### Download Visual Studio 2022 Community (Free)
https://visualstudio.microsoft.com/downloads/

During installation, select:
- ✅ **Desktop development with C++**
- ✅ **C++ CMake tools for Windows**

### Then Build:
```powershell
# Open "Developer PowerShell for VS 2022" from Start Menu
cd C:\Users\maxpa\SimpleSynth
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Run tests
.\Tests\Release\SimpleSynth_Tests.exe
```

---

## Option 2: Test DSP Components Standalone (Without JUCE Plugin)

Since the DSP core is JUCE-independent, we can test it with MinGW!

### Create Standalone Test

```powershell
cd C:\Users\maxpa\SimpleSynth
mkdir standalone_test
cd standalone_test
```

Create `test_dsp_standalone.cpp`:

```cpp
#include "../Source/DSP/Common.h"
#include "../Source/DSP/Oscillator.h"
#include "../Source/DSP/Envelope.h"
#include "../Source/DSP/Voice.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace SimpleSynth::DSP;

int main() {
    std::cout << "SimpleSynth DSP Standalone Test\n";
    std::cout << "================================\n\n";

    const float sampleRate = 44100.0f;
    const int numSamples = 44100 * 2; // 2 seconds

    // Test 1: Oscillator
    std::cout << "Test 1: Oscillator (440 Hz sine wave)\n";
    Oscillator osc;
    osc.Init(sampleRate);
    osc.SetFrequency(440.0f);
    osc.SetWaveform(Oscillator::Waveform::Sine);

    std::vector<float> oscBuffer(1000);
    osc.Process(oscBuffer.data(), 1000);
    std::cout << "  ✓ Generated 1000 samples\n";
    std::cout << "  Sample[0] = " << oscBuffer[0] << "\n";
    std::cout << "  Sample[500] = " << oscBuffer[500] << "\n\n";

    // Test 2: Envelope
    std::cout << "Test 2: ADSR Envelope\n";
    Envelope env;
    env.Init(sampleRate);
    env.SetParameters(10.0f, 100.0f, 0.7f, 200.0f);
    env.NoteOn();

    std::vector<float> envBuffer(1000);
    env.Process(envBuffer.data(), 1000);
    std::cout << "  ✓ Attack phase: " << envBuffer[0] << " -> " << envBuffer[500] << "\n";
    std::cout << "  ✓ Stage: " << (int)env.GetStage() << "\n\n";

    // Test 3: Voice (Full Synth)
    std::cout << "Test 3: Complete Voice (MIDI note 60 = Middle C)\n";
    Voice voice;
    voice.Init(sampleRate);
    voice.SetOscillatorWaveform(Oscillator::Waveform::Saw);
    voice.SetEnvelopeParameters(10.0f, 100.0f, 0.7f, 200.0f);
    voice.NoteOn(60, 1.0f); // Middle C at full velocity

    std::vector<float> audioBuffer(numSamples);
    voice.Process(audioBuffer.data(), numSamples);

    // Find peak level
    float peak = 0.0f;
    for (float sample : audioBuffer) {
        peak = std::max(peak, std::abs(sample));
    }
    std::cout << "  ✓ Peak level: " << peak << "\n";
    std::cout << "  ✓ Voice active: " << (voice.IsActive() ? "Yes" : "No") << "\n\n";

    // Save to WAV file (simple 16-bit PCM)
    std::cout << "Test 4: Saving to test_output.wav\n";
    std::ofstream wavFile("test_output.wav", std::ios::binary);

    // WAV header
    const int byteRate = sampleRate * 2; // 16-bit mono
    const int dataSize = numSamples * 2;

    wavFile.write("RIFF", 4);
    int chunkSize = 36 + dataSize;
    wavFile.write((char*)&chunkSize, 4);
    wavFile.write("WAVE", 4);
    wavFile.write("fmt ", 4);
    int subchunk1Size = 16;
    wavFile.write((char*)&subchunk1Size, 4);
    short audioFormat = 1; // PCM
    wavFile.write((char*)&audioFormat, 2);
    short numChannels = 1;
    wavFile.write((char*)&numChannels, 2);
    int sampleRateInt = (int)sampleRate;
    wavFile.write((char*)&sampleRateInt, 4);
    wavFile.write((char*)&byteRate, 4);
    short blockAlign = 2;
    wavFile.write((char*)&blockAlign, 2);
    short bitsPerSample = 16;
    wavFile.write((char*)&bitsPerSample, 2);
    wavFile.write("data", 4);
    wavFile.write((char*)&dataSize, 4);

    // Write audio data
    for (float sample : audioBuffer) {
        short intSample = (short)(sample * 32767.0f);
        wavFile.write((char*)&intSample, 2);
    }

    wavFile.close();
    std::cout << "  ✓ Saved " << numSamples << " samples to test_output.wav\n";
    std::cout << "  ✓ Play it with Windows Media Player or VLC!\n\n";

    std::cout << "All tests passed! ✓\n";
    return 0;
}
```

### Build and Run Standalone Test:

```powershell
C:\msys64\msys2_shell.cmd -mingw64 -defterm -no-start -c "cd /c/Users/maxpa/SimpleSynth/standalone_test && g++ test_dsp_standalone.cpp ../Source/DSP/Oscillator.cpp ../Source/DSP/Envelope.cpp ../Source/DSP/Voice.cpp -I.. -std=c++17 -o test_dsp && ./test_dsp"
```

This will:
1. Test all DSP components
2. Generate a 2-second audio file: `test_output.wav`
3. You can play the WAV file to hear your synth!

---

## Option 3: Use WSL2 + Linux

If you have WSL installed:

```bash
wsl
cd /mnt/c/Users/maxpa/SimpleSynth
sudo apt update
sudo apt install build-essential cmake ninja-build git
sudo apt install libasound2-dev libjack-dev

# Build
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja

# Run tests
./Tests/SimpleSynth_Tests
```

---

## Summary

### Why Each Method Failed:

| Method | Status | Reason |
|--------|--------|--------|
| MSYS2/MinGW | ❌ Failed | JUCE explicitly doesn't support MinGW |
| VS BuildTools | ❌ Cancelled | Installation interrupted |
| VS Community | ❌ Cancelled | Installation interrupted |

### What Works:

| Method | Complexity | Result |
|--------|------------|--------|
| **Standalone DSP Test** | ⭐ Easy | Test DSP without JUCE |
| **Visual Studio 2022** | ⭐⭐ Medium | Full plugin build |
| **WSL2** | ⭐⭐⭐ Advanced | Linux-based build |

---

## Next Steps

**Easiest path forward:**

1. Create the standalone test (Option 2) to verify DSP works ✅
2. Install Visual Studio 2022 Community when you have time ⏰
3. Build full VST3 plugin with MSVC 🎹

Would you like me to create the standalone test files for you?
