#ifdef FLIGHT_RECORDER

#pragma once

#include "System/Capture/ImageOps.h"

#include <deque>
#include <string>
#include <vector>
#include <cstdint>

namespace AV{

    /**
    A variable the game asked the recorder to sample every frame, and its value that frame.
    */
    struct WatchValue{
        std::string name;
        std::string value;
    };

    /**
    Everything the recorder knows about one rendered frame.

    The pixels are already downsampled to the capture resolution by the time they get here;
    the full resolution frame is kept separately and only for the frame a capture lands on.
    */
    struct FrameRecord{
        uint64_t frameNumber = 0;
        //Milliseconds since recording started.
        double timeMs = 0.0;
        float frameTimeMs = 0.0f;
        float fps = 0.0f;
        //Perceptual hash of the downsampled frame, so an agent can spot where the image changed.
        uint64_t dHash = 0;

        /**
        Half open range of absolute script event indices belonging to this frame. Absolute
        rather than ring relative, so a slice can be recognised as having been overwritten
        once the event ring wraps past it.
        */
        uint64_t scriptEventBegin = 0;
        uint64_t scriptEventEnd = 0;
        //Function indices of the deepest squirrel call stack reached during this frame.
        std::vector<uint32_t> deepestStack;

        std::vector<std::string> marks;
        std::vector<WatchValue> watches;

        CapturedFrame frame;

        //Bytes of pixel data this record holds.
        size_t byteSize() const { return frame.rgb.size(); }
    };

    /**
    Circular buffer of the most recent frames.

    Bounded twice over: by a frame count and by a total pixel budget, evicting oldest first
    on whichever binds. Holds no Ogre or squirrel state, so it is unit testable with
    synthetic records.
    */
    class FrameRing{
    public:
        FrameRing() = default;

        /**
        Set the bounds and drop anything already held.
        */
        void configure(uint32_t capacityFrames, size_t maxBytes);

        /**
        Append a frame, evicting the oldest frames until both bounds are satisfied.

        A single record larger than the whole byte budget is still kept - refusing it would
        leave the ring permanently empty, which is worse than briefly exceeding the budget.
        */
        void push(FrameRecord&& record);

        /**
        Move every frame out, oldest first, leaving the ring empty and ready to record again.
        This is what a capture does: the writer thread takes ownership of the pixels rather
        than copying them.
        */
        std::vector<FrameRecord> takeAll();

        void clear();

        size_t size() const { return mFrames.size(); }
        bool empty() const { return mFrames.empty(); }
        size_t totalBytes() const { return mTotalBytes; }
        uint32_t capacity() const { return mCapacity; }

        /**
        Oldest first access, for inspection without taking ownership.
        */
        const FrameRecord& at(size_t index) const { return mFrames.at(index); }

    private:
        std::deque<FrameRecord> mFrames;
        uint32_t mCapacity = 100;
        size_t mMaxBytes = 256 * 1024 * 1024;
        size_t mTotalBytes = 0;
    };
}

#endif
