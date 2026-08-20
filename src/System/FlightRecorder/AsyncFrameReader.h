#ifdef FLIGHT_RECORDER

#pragma once

#include "System/Capture/ImageOps.h"

#include "OgrePixelFormatGpu.h"

#include <string>
#include <vector>
#include <cstdint>

namespace Ogre{
    class AsyncTextureTicket;
    class TextureGpu;
}

namespace AV{

    /**
    Reads the colour buffer back from the GPU without stalling on it.

    The debug server's FrameCapture reads back synchronously, which is right for a one off
    request but far too expensive to do on every frame: measured on a headless run it cost
    roughly five sixths of the frame rate, because each readback flushes the pipeline and
    waits for the GPU to catch up.

    This instead keeps a small pool of Ogre async tickets. Each frame one ticket is asked to
    download the colour buffer and the request is left to complete in the background; a
    ticket queued several frames earlier - by which time the GPU is long done with it - is
    mapped and copied out. The result is a capture that lags the live frame by the pool depth,
    which for a recorder is irrelevant: every frame still reaches the ring, just slightly later.

    One complication: a ticket cannot download directly from a window's swapchain texture.
    On Metal that blit dereferences the drawable's absent backing resource and segfaults
    inside the driver. So when the source is render-window specific the frame is first copied
    on the GPU into an intermediate offscreen texture - cheap, and a copy the driver is happy
    to do - and the ticket downloads from that instead.

    Falls back to the synchronous path when tickets or the intermediate cannot be created, so
    a render system that cannot do this still records.
    */
    class AsyncFrameReader{
    public:
        AsyncFrameReader() = default;
        ~AsyncFrameReader();

        /**
        Allocate the ticket pool for a texture of this size and format. Safe to call again
        when the window is resized; the pool is rebuilt.
        */
        bool initialise(uint32_t width, uint32_t height, Ogre::PixelFormatGpu format);

        void shutdown();

        /**
        Queue a download of the given texture, and copy out whichever earlier download has
        since completed.

        Must be called from frameRenderingQueued, for the drawable lifetime reason described
        in FrameCapture.

        @param frameNumber Identifies the frame being queued now. The frame handed back in
                           out carries the number it was queued with, not this one.
        @param out Receives a completed frame.
        @return True if out was filled. False while the pool is still filling up, which is
                normal for the first few frames.
        */
        bool submitAndCollect(Ogre::TextureGpu* texture, uint64_t frameNumber,
                              CapturedFrame& out, std::string& outError);

        bool isReady() const { return mReady; }

        //Frames a capture lags the live frame by.
        static const size_t POOL_SIZE = 3;

    private:
        struct Slot{
            Ogre::AsyncTextureTicket* ticket = nullptr;
            bool pending = false;
            uint64_t frameNumber = 0;
        };

        //Copies a mapped ticket into a tightly packed rgb frame.
        static void _copyOut(const Ogre::TextureBox& box, Ogre::PixelFormatGpu format,
                             uint32_t width, uint32_t height, CapturedFrame& out);

        std::vector<Slot> mSlots;
        size_t mNext = 0;
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;
        Ogre::PixelFormatGpu mFormat = Ogre::PFG_UNKNOWN;
        bool mReady = false;
    };
}

#endif
