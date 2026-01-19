/*
  Thread-Safe Ring Buffer
  For passing IQ samples from SDR thread to audio thread

  Inspired by:
  - SDRPlusPlus ring buffer implementation
  - JUCE AbstractFifo pattern
*/

#pragma once
#include <vector>
#include <atomic>

template<typename T>
class RingBuffer
{
public:
    explicit RingBuffer(size_t capacity = 65536)
        : buffer(capacity)
        , size(capacity)
    {
        readPos.store(0);
        writePos.store(0);
    }

    // Write data to the buffer (returns number of samples actually written)
    size_t write(const T* data, size_t count)
    {
        size_t available = getWriteAvailable();
        size_t toWrite = std::min(count, available);

        if (toWrite == 0)
            return 0;

        size_t wp = writePos.load(std::memory_order_acquire);

        // Handle wrap-around
        size_t firstPart = std::min(toWrite, size - wp);
        std::memcpy(buffer.data() + wp, data, firstPart * sizeof(T));

        if (firstPart < toWrite)
        {
            size_t secondPart = toWrite - firstPart;
            std::memcpy(buffer.data(), data + firstPart, secondPart * sizeof(T));
        }

        writePos.store((wp + toWrite) % size, std::memory_order_release);
        return toWrite;
    }

    // Read data from the buffer
    size_t read(T* data, size_t count)
    {
        size_t available = getReadAvailable();
        size_t toRead = std::min(count, available);

        if (toRead == 0)
            return 0;

        size_t rp = readPos.load(std::memory_order_acquire);

        // Handle wrap-around
        size_t firstPart = std::min(toRead, size - rp);
        std::memcpy(data, buffer.data() + rp, firstPart * sizeof(T));

        if (firstPart < toRead)
        {
            size_t secondPart = toRead - firstPart;
            std::memcpy(data + firstPart, buffer.data(), secondPart * sizeof(T));
        }

        readPos.store((rp + toRead) % size, std::memory_order_release);
        return toRead;
    }

    // Skip samples without reading
    void skip(size_t count)
    {
        size_t available = getReadAvailable();
        size_t toSkip = std::min(count, available);

        if (toSkip > 0)
        {
            size_t rp = readPos.load(std::memory_order_acquire);
            readPos.store((rp + toSkip) % size, std::memory_order_release);
        }
    }

    size_t getReadAvailable() const
    {
        size_t wp = writePos.load(std::memory_order_acquire);
        size_t rp = readPos.load(std::memory_order_acquire);

        if (wp >= rp)
            return wp - rp;
        else
            return size - rp + wp;
    }

    size_t getWriteAvailable() const
    {
        return size - getReadAvailable() - 1; // -1 to avoid wrap ambiguity
    }

    void clear()
    {
        readPos.store(0);
        writePos.store(0);
    }

private:
    std::vector<T> buffer;
    size_t size;
    std::atomic<size_t> readPos;
    std::atomic<size_t> writePos;
};
