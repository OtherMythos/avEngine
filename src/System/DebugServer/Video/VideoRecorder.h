#ifdef DEBUG_SERVER
#pragma once

#include "System/DebugServer/Render/ImageOps.h"
#include "OgreFrameListener.h"

#include <condition_variable>
#include <atomic>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

namespace AV{
    class VideoRecorder : public Ogre::FrameListener{
    public:
        VideoRecorder();
        ~VideoRecorder();

        bool start(const std::string& outputDir, int fps, uint64 frameLimit,
                   int width, int height, uint64 startTick, int tickRate, std::string& error);
        void stop();
        void shutdown();
        bool active() const { return mActive; }
        int ticksUntilFrame(uint64 tick) const;
        bool atFrameBoundary(uint64 tick) const;
        void prepareFrame(uint64 tick);
        bool frameRenderingQueued(const Ogre::FrameEvent& event) override;

        struct Status{
            std::string state;
            std::string outputPath;
            std::string lastError;
            int fps = 0;
            uint64 frameLimit = 0;
            uint64 framesCaptured = 0;
            uint64 framesEncoded = 0;
            size_t queued = 0;
        };
        Status status() const;

    private:
        void encode();
        void fail(const std::string& error);

        mutable std::mutex mMutex;
        std::condition_variable mCv;
        std::deque<CapturedFrame> mQueue;
        std::thread mWorker;
        bool mRegistered = false;
        std::atomic<bool> mActive{false};
        bool mCapturePending = false;
        bool mStopping = false;
        bool mReachedLimit = false;
        std::string mState = "idle";
        std::string mOutputPath;
        std::string mLastError;
        int mFps = 0;
        int mTicksPerFrame = 0;
        int mWidth = 0;
        int mHeight = 0;
        uint64 mStartTick = 0;
        uint64 mFrameLimit = 0;
        uint64 mFramesCaptured = 0;
        uint64 mFramesEncoded = 0;
    };
}
#endif
