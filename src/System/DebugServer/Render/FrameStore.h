#ifdef DEBUG_SERVER

#pragma once

#include "ImageOps.h"

#include <map>
#include <string>
#include <vector>
#include <deque>
#include <mutex>

namespace AV{
    /**
    Named snapshots of captured frames, so an agent can compare "before" and "after"
    without ever holding pixels itself.

    Frames are stored at analysis resolution (the caller downsamples before saving), so
    each slot is a couple of hundred KB rather than a full retina capture. The number of
    slots is capped; saving past the cap evicts the oldest.

    Thread safety: unlike the other debug-server state this is guarded by its own mutex,
    because frame capture (and therefore snapshotting) runs on the HTTP thread rather
    than the main-thread pump - see FrameCapture for why.
    */
    class FrameStore{
    public:
        void save(const std::string& name, const CapturedFrame& frame);

        /**
        Copy out a stored frame.
        @return False if no such slot.
        */
        bool get(const std::string& name, CapturedFrame& out) const;

        bool remove(const std::string& name);
        void clear();

        std::vector<std::string> names() const;
        size_t size() const;

        static const size_t MAX_SLOTS = 16;

    private:
        mutable std::mutex mMutex;
        std::map<std::string, CapturedFrame> mSlots;
        //Insertion order, for evicting the oldest slot when full.
        std::deque<std::string> mOrder;
    };
}

#endif
