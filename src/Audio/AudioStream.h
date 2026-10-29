#pragma once

#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>

#include "sndfile.h"

namespace AV{
    class AudioStream : public std::enable_shared_from_this<AudioStream>{
    public:
        static constexpr size_t BlockFrames = 8192;
        static constexpr size_t BlockCount = 4;
        struct Block{
            std::array<short, BlockFrames * 2> samples;
            size_t frames = 0;
            int64_t firstFrame = 0;
        };
        struct Status{
            int rate = 0, channels = 0;
            int64_t frames = 0;
            size_t readyFrames = 0;
            bool eof = false, pending = false;
            std::string error;
        };

        explicit AudioStream(const std::string& path);
        ~AudioStream();
        void reset(double seconds, bool looping, bool active = true);
        void cancel();
        void schedule();
        Status status() const;
        //The consumer owns this slot until release(); reset never recycles it.
        const Block* acquire(size_t& slot);
        void release(size_t slot);
        bool wait(std::chrono::milliseconds timeout);
        static size_t memoryBytes() { return sizeof(Block) * BlockCount; }

    private:
        class DecodeJob;
        void decode();
        void jobFinished();
        const std::string mPath;
        mutable std::mutex mMutex;
        std::condition_variable mChanged;
        std::array<Block, BlockCount> mBlocks;
        std::deque<size_t> mFree, mReady;
        uint64_t mGeneration = 0, mDecoderGeneration = 0;
        double mSeekSeconds = 0;
        bool mLooping = false, mActive = false, mCancelled = false, mPending = false;
        Status mStatus;
        //Only the one outstanding decode job touches the file and its cursor.
        SNDFILE* mFile = nullptr;
        int mRate = 0, mChannels = 0;
        int64_t mFrames = 0, mCursor = 0;
    };
}
