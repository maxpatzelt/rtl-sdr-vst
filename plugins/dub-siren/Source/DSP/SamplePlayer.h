#pragma once
#include <cstddef>
#include <cmath>

namespace DubSiren::DSP {

/**
 * One-shot / looping sample player with fractional playback rate.
 * No heap allocation in audio thread — Load() points at a pre-decoded buffer.
 * SetPlaybackRate() controls speed (0.1 = very slow, 4.0 = 4× speed).
 * Linear interpolation for smooth non-integer positions.
 * Loop mode: sample restarts automatically at end until Stop() called.
 */
class SamplePlayer
{
public:
    SamplePlayer() = default;

    // Point at decoded mono float data (pre-allocated by caller, e.g. AudioBuffer).
    void Load(const float* data, size_t numFrames)
    {
        data_      = data;
        numFrames_ = numFrames;
        readPos_   = static_cast<float>(numFrames);  // start silent
        playing_   = false;
    }

    void Trigger()
    {
        if (data_ == nullptr || numFrames_ == 0) return;
        readPos_ = 0.0f;
        playing_ = true;
    }

    void Stop()  { playing_ = false; }

    void SetPlaybackRate(float rate) { playbackRate_ = rate < 0.05f ? 0.05f : rate; }
    void SetLoop(bool loop)          { loop_ = loop; }
    bool IsPlaying() const           { return playing_; }

    float ProcessSample()
    {
        if (!playing_ || data_ == nullptr) return 0.0f;

        const auto iPos  = static_cast<size_t>(readPos_);
        const float frac = readPos_ - static_cast<float>(iPos);

        if (iPos >= numFrames_)
        {
            if (loop_) { readPos_ = 0.0f; return 0.0f; }
            playing_  = false;
            return 0.0f;
        }

        const size_t i1  = (iPos + 1 < numFrames_) ? iPos + 1 : iPos;
        const float  out = data_[iPos] * (1.0f - frac) + data_[i1] * frac;

        readPos_ += playbackRate_;
        return out;
    }

private:
    const float* data_         = nullptr;
    size_t       numFrames_    = 0;
    float        readPos_      = 0.0f;
    float        playbackRate_ = 1.0f;
    bool         playing_      = false;
    bool         loop_         = false;
};

} // namespace DubSiren::DSP
