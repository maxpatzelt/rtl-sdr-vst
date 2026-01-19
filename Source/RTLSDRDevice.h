/*
  RTL-SDR Device Manager
  Handles device enumeration, initialization, tuning, and IQ sample streaming

  Architecture inspired by:
  - librtlsdr: Device management and control
  - SDRPlusPlus: Modular device handling
  - rtl_fm_player: Sample callback patterns
*/

#pragma once
#include <juce_core/juce_core.h>
#include <atomic>
#include <memory>
#include <vector>
#include <thread>

#if HAS_RTLSDR
#include <rtl-sdr.h>
#endif

class RTLSDRDevice
{
public:
    struct DeviceInfo
    {
        int index;
        juce::String name;
        juce::String serial;
    };

    // Callback for IQ samples (interleaved I/Q uint8 data)
    using SampleCallback = std::function<void(const uint8_t* data, uint32_t length)>;

    RTLSDRDevice();
    ~RTLSDRDevice();

    // Device enumeration
    std::vector<DeviceInfo> enumerateDevices();

    // Device lifecycle
    bool openDevice(int deviceIndex = 0);
    void closeDevice();
    bool isOpen() const { return deviceOpen.load(); }

    // Tuning and configuration
    bool setFrequency(uint32_t frequencyHz);
    bool setSampleRate(uint32_t sampleRateHz);
    bool setGain(int gainTenthsDB); // Gain in tenths of dB (e.g., 290 = 29.0 dB)
    bool setAutoGain(bool enable);

    uint32_t getFrequency() const { return currentFrequency.load(); }
    uint32_t getSampleRate() const { return currentSampleRate.load(); }

    // Streaming control
    bool startStreaming(SampleCallback callback);
    void stopStreaming();
    bool isStreaming() const { return streaming.load(); }

    // Error info
    juce::String getLastError() const { return lastError; }

private:
    void streamingThread();
    static void rtlsdrCallback(unsigned char* buf, uint32_t len, void* ctx);

#if HAS_RTLSDR
    void* device = nullptr;
#endif

    std::atomic<bool> deviceOpen{false};
    std::atomic<bool> streaming{false};
    std::atomic<bool> shouldStop{false};

    std::atomic<uint32_t> currentFrequency{100000000}; // 100 MHz default
    std::atomic<uint32_t> currentSampleRate{1024000};  // 1.024 MHz default

    SampleCallback sampleCallback;
    std::unique_ptr<std::thread> streamThread;
    juce::String lastError;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RTLSDRDevice)
};
