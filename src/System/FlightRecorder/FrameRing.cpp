#ifdef FLIGHT_RECORDER

#include "FrameRing.h"

namespace AV{

    void FrameRing::configure(uint32_t capacityFrames, size_t maxBytes){
        mCapacity = capacityFrames > 0 ? capacityFrames : 1;
        mMaxBytes = maxBytes;
        clear();
    }

    void FrameRing::push(FrameRecord&& record){
        const size_t incoming = record.byteSize();

        mFrames.push_back(std::move(record));
        mTotalBytes += incoming;

        while(mFrames.size() > mCapacity){
            mTotalBytes -= mFrames.front().byteSize();
            mFrames.pop_front();
        }
        //Never evict the frame just pushed; a record bigger than the entire budget would
        //otherwise empty the ring on every push.
        while(mTotalBytes > mMaxBytes && mFrames.size() > 1){
            mTotalBytes -= mFrames.front().byteSize();
            mFrames.pop_front();
        }
    }

    std::vector<FrameRecord> FrameRing::takeAll(){
        std::vector<FrameRecord> out;
        out.reserve(mFrames.size());
        for(FrameRecord& f : mFrames){
            out.push_back(std::move(f));
        }
        clear();
        return out;
    }

    void FrameRing::clear(){
        mFrames.clear();
        mTotalBytes = 0;
    }
}

#endif
