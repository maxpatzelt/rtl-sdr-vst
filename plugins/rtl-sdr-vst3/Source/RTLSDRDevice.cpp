/*
  RTL-SDR Device Implementation
  Based on patterns from librtlsdr examples and rtl_fm
*/

#include "RTLSDRDevice.h"

RTLSDRDevice::RTLSDRDevice()
{
}

RTLSDRDevice::~RTLSDRDevice()
{
    stopStreaming();
    closeDevice();
}

std::vector<RTLSDRDevice::DeviceInfo> RTLSDRDevice::enumerateDevices()
{
    std::vector<DeviceInfo> devices;

#if HAS_RTLSDR
    uint32_t deviceCount = rtlsdr_get_device_count();

    for (uint32_t i = 0; i < deviceCount; i++)
    {
        DeviceInfo info;
        info.index = i;
        info.name = rtlsdr_get_device_name(i);

        char manufacturer[256], product[256], serial[256];
        if (rtlsdr_get_device_usb_strings(i, manufacturer, product, serial) == 0)
        {
            info.serial = serial;
        }

        devices.push_back(info);
    }
#else
    // Dummy device for testing without hardware
    DeviceInfo dummy;
    dummy.index = 0;
    dummy.name = "RTL-SDR (Hardware Not Available)";
    dummy.serial = "DUMMY";
    devices.push_back(dummy);
#endif

    return devices;
}

bool RTLSDRDevice::openDevice(int deviceIndex)
{
    if (deviceOpen.load())
    {
        lastError = "Device already open";
        return false;
    }

#if HAS_RTLSDR
    int result = rtlsdr_open(&device, deviceIndex);
    if (result < 0)
    {
        lastError = "Failed to open RTL-SDR device: " + juce::String(result);
        return false;
    }

    // Set reasonable defaults
    rtlsdr_set_sample_rate(device, currentSampleRate.load());
    rtlsdr_set_center_freq(device, currentFrequency.load());
    rtlsdr_set_tuner_gain_mode(device, 0); // Auto gain
    rtlsdr_reset_buffer(device);

    deviceOpen.store(true);
    lastError = "";
    return true;
#else
    // Simulate device opening
    deviceOpen.store(true);
    lastError = "Running in simulation mode (no hardware)";
    return true;
#endif
}

void RTLSDRDevice::closeDevice()
{
    stopStreaming();

    if (!deviceOpen.load())
        return;

#if HAS_RTLSDR
    if (device)
    {
        rtlsdr_close(device);
        device = nullptr;
    }
#endif

    deviceOpen.store(false);
}

bool RTLSDRDevice::setFrequency(uint32_t frequencyHz)
{
    currentFrequency.store(frequencyHz);

    if (!deviceOpen.load())
        return false;

#if HAS_RTLSDR
    int result = rtlsdr_set_center_freq(device, frequencyHz);
    if (result < 0)
    {
        lastError = "Failed to set frequency";
        return false;
    }
#endif

    return true;
}

bool RTLSDRDevice::setSampleRate(uint32_t sampleRateHz)
{
    currentSampleRate.store(sampleRateHz);

    if (!deviceOpen.load())
        return false;

#if HAS_RTLSDR
    int result = rtlsdr_set_sample_rate(device, sampleRateHz);
    if (result < 0)
    {
        lastError = "Failed to set sample rate";
        return false;
    }
#endif

    return true;
}

bool RTLSDRDevice::setGain(int gainTenthsDB)
{
    if (!deviceOpen.load())
        return false;

#if HAS_RTLSDR
    int result = rtlsdr_set_tuner_gain(device, gainTenthsDB);
    if (result < 0)
    {
        lastError = "Failed to set gain";
        return false;
    }
#endif

    return true;
}

bool RTLSDRDevice::setAutoGain(bool enable)
{
    if (!deviceOpen.load())
        return false;

#if HAS_RTLSDR
    int result = rtlsdr_set_tuner_gain_mode(device, enable ? 0 : 1);
    if (result < 0)
    {
        lastError = "Failed to set auto gain mode";
        return false;
    }
#endif

    return true;
}

bool RTLSDRDevice::startStreaming(SampleCallback callback)
{
    if (!deviceOpen.load())
    {
        lastError = "Device not open";
        return false;
    }

    if (streaming.load())
    {
        lastError = "Already streaming";
        return false;
    }

    sampleCallback = callback;
    shouldStop.store(false);
    streaming.store(true);

#if HAS_RTLSDR
    rtlsdr_reset_buffer(device);

    // Start async read loop in separate thread
    // Pattern from rtl_fm and rtl_ais
    streamThread = std::make_unique<std::thread>([this]()
    {
        // Read samples continuously
        // Using synchronous read for simplicity (async is more complex)
        const int bufferSize = 16384; // 16K buffer
        std::vector<uint8_t> buffer(bufferSize);

        while (!shouldStop.load() && deviceOpen.load())
        {
            int n_read = 0;
            int result = rtlsdr_read_sync(device, buffer.data(), bufferSize, &n_read);

            if (result < 0 || n_read == 0)
                break;

            if (sampleCallback && n_read > 0)
            {
                sampleCallback(buffer.data(), n_read);
            }
        }

        streaming.store(false);
    });
#else
    // Generate dummy IQ data for testing
    streamThread = std::make_unique<std::thread>([this]()
    {
        const int bufferSize = 16384;
        std::vector<uint8_t> buffer(bufferSize);

        // Generate simple test pattern (centered at 127)
        while (!shouldStop.load())
        {
            for (int i = 0; i < bufferSize; i++)
            {
                buffer[i] = 127 + (uint8_t)(30.0f * std::sin(i * 0.1f));
            }

            if (sampleCallback)
            {
                sampleCallback(buffer.data(), bufferSize);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate timing
        }

        streaming.store(false);
    });
#endif

    return true;
}

void RTLSDRDevice::stopStreaming()
{
    if (!streaming.load())
        return;

    shouldStop.store(true);

#if HAS_RTLSDR
    if (device)
    {
        rtlsdr_cancel_async(device);
    }
#endif

    if (streamThread)
    {
        if (streamThread->joinable())
            streamThread->join();
        streamThread.reset();
    }

    streaming.store(false);
}
