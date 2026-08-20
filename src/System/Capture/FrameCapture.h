#if defined(DEBUG_SERVER) || defined(FLIGHT_RECORDER)

#pragma once

#include "ImageOps.h"

#include "OgreFrameListener.h"

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <cstdint>

namespace AV{
    /**
    Reads the engine's final colour buffer (Window::getRenderTexture) back from the GPU
    into a CapturedFrame.

    Timing is critical on the windowed path with Metal: the window drawable is released
    when the frame is presented, so reading it after renderOneFrame() dereferences a dead
    texture and crashes. The only valid window is mid-frame — after render commands are
    issued but before the final swap. Ogre fires FrameListener::frameRenderingQueued in
    exactly that window (see Root::_updateAllRenderTargets: compositor update, then the
    listeners, then _swapAllFinalTargets).

    Headless the target is a plain offscreen texture with no drawable, so that constraint
    doesn't apply — but the timing is kept identical so both paths share one code path.

    This class is therefore a FrameListener rather than a MainThreadQueue consumer:
    an HTTP thread calls requestCapture() and blocks; the next frameRenderingQueued
    (main thread, drawable alive) performs the synchronous readback and wakes it. The
    caller then owns a CPU-side copy it can process on its own thread.
    */
    class FrameCapture : public Ogre::FrameListener{
    public:
        /**
        Read the engine's final colour buffer back into out.

        Static and public because more than one subsystem needs it: the debug server captures
        on demand, the flight recorder captures every frame. Both must call it from inside
        frameRenderingQueued for the drawable-lifetime reason described above.

        @return False on failure, with outError describing why.
        */
        static bool readColourBuffer(CapturedFrame& out, std::string& outError);

        FrameCapture() = default;
        virtual ~FrameCapture() = default;

        /**
        Register with Ogre::Root as a frame listener. Call on the main thread once Ogre exists.
        */
        void initialise();

        /**
        Unregister and wake any blocked requesters (they receive failure). Must be
        called before the HTTP server stops, for the same deadlock reason as
        MainThreadQueue::shutdown.
        */
        void shutdown();

        /**
        Block until the next rendered frame has been captured, or timeoutMs elapses
        (engine paused/backgrounded/shutting down).

        Called from HTTP threads. On success out holds the frame and its frame number.
        */
        bool requestCapture(CapturedFrame& out, std::string& outError, uint32_t timeoutMs);

        //Ogre::FrameListener. Runs on the main thread, before the final swap.
        bool frameRenderingQueued(const Ogre::FrameEvent& evt) override;

    private:
        //Performs the actual readback. Returns false and fills mError on failure.
        bool _performCapture();

        std::mutex mMutex;
        std::condition_variable mCv;
        bool mPending = false;
        bool mDone = false;
        bool mSuccess = false;
        CapturedFrame mFrame;
        std::string mError;

        //Counts rendered frames. Written only in frameRenderingQueued.
        uint64_t mFrameNumber = 0;

        std::atomic<bool> mShutdown{false};
        bool mRegistered = false;
    };
}

#endif
