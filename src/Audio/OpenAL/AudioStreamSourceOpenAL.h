#pragma once

#include "AudioSourceOpenAL.h"
#include "Audio/AudioStream.h"
#include <deque>

namespace AV{
    class AudioStreamSourceOpenAL : public AudioSourceOpenAL{
    public:
        AudioStreamSourceOpenAL(AudioManager* manager, const std::string& path);
        ~AudioStreamSourceOpenAL() override;
        void releaseBackend() override;
        bool isStream() const override { return true; }
        void setAudioBuffer(AudioBufferPtr buffer) override;
        void play() override;
        void pause() override;
        void stop() override;
        void setLooping(bool looping) override;
        void seekSeconds(float seconds) override;
        std::string getPlaybackState() const override;
        double getDurationSeconds() const override;
        double getPositionSeconds() const override;
        double getBufferedSeconds() const override;
        std::string getError() const override;
        std::string getAudioPath() const override { return mPath; }
        unsigned getUnderruns() const override { return mUnderruns; }
        size_t getStreamMemoryBytes() const override { return AudioStream::memoryBytes() + AudioStream::BlockCount * AudioStream::BlockFrames * 2 * sizeof(short); }
        double getDecodedSeconds() const;
        void update();
        void prepareSimulation();
#ifdef DEBUG_SERVER
        AudioSourceSnapshot debugSnapshot() const override;
        std::string debugPlaybackState() const override { return getPlaybackState(); }
#endif

    private:
        enum class Intent { Ready, Playing, Paused, Stopped };
        struct QueuedBuffer{
            ALuint buffer;
            size_t frames;
            int64_t firstFrame;
        };
        void flush();
        void reset(double seconds, bool active);
        void fail(const std::string& error);
        const std::string mPath;
        std::shared_ptr<AudioStream> mStream;
        std::array<ALuint, AudioStream::BlockCount> mBuffers{};
        std::deque<ALuint> mFree;
        std::deque<QueuedBuffer> mQueue;
        AudioStream::Status mStatus;
        Intent mIntent = Intent::Ready;
        bool mRewindOnPlay = false;
        bool mLooping = false, mStarted = false, mStarved = false, mEnded = false;
        unsigned mUnderruns = 0;
        double mPosition = 0;
        std::string mError;
    };
}
