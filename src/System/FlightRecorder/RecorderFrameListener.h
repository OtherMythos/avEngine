#ifdef FLIGHT_RECORDER

#pragma once

#include "System/Capture/ImageOps.h"
#include "AsyncFrameReader.h"

#include "OgreFrameListener.h"

#include <atomic>
#include <cstdint>

namespace AV{

    /**
    Drives the recorder's per frame colour buffer readback.

    Registered with Ogre::Root rather than called from Base::update because the only point at
    which the window's colour buffer can be read is mid frame, after render commands are
    issued but before the final swap: on Metal the drawable is released when the frame is
    presented, so a readback afterwards dereferences a dead texture. Ogre fires
    frameRenderingQueued in exactly that window. See FrameCapture for the full explanation.

    Everything here runs on the main thread.
    */
    class RecorderFrameListener : public Ogre::FrameListener{
    public:
        RecorderFrameListener() = default;
        virtual ~RecorderFrameListener() = default;

        void initialise(uint32_t captureWidth, uint32_t captureHeight, uint32_t everyNthFrame, bool fastSample);
        void shutdown();

        /**
        Ask for the next rendered frame to also be kept at full resolution, for the frame a
        capture lands on. Cleared once that frame has been read.
        */
        void requestFullFrame() { mWantFullFrame = true; }

        /**
        The number of frames rendered since the recorder started. Frames are named by this.
        */
        uint64_t frameNumber() const { return mFrameNumber; }

        bool frameRenderingQueued(const Ogre::FrameEvent& evt) override;

    private:
        uint32_t mCaptureWidth = 480;
        uint32_t mCaptureHeight = 270;
        uint32_t mEveryNthFrame = 1;
        //Point sample rather than box average when reducing a frame. Much cheaper, slightly aliased.
        bool mFastSample = false;

        uint64_t mFrameNumber = 0;
        bool mWantFullFrame = false;
        bool mRegistered = false;

        //Non stalling readback. Falls back to the synchronous path when it cannot be set up.
        AsyncFrameReader mAsyncReader;
        bool mAsyncAttempted = false;
        //Readback failures are logged once rather than every frame; a broken render target
        //stays broken and would otherwise flood the log at frame rate.
        bool mLoggedFailure = false;
    };
}

#endif
