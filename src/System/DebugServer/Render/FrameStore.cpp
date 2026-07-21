#ifdef DEBUG_SERVER

#include "FrameStore.h"

#include <algorithm>

namespace AV{
    void FrameStore::save(const std::string& name, const CapturedFrame& frame){
        std::lock_guard<std::mutex> lock(mMutex);

        const bool isNew = mSlots.find(name) == mSlots.end();
        if(isNew){
            //Evict the oldest slot to stay bounded.
            while(mOrder.size() >= MAX_SLOTS){
                const std::string oldest = mOrder.front();
                mOrder.pop_front();
                mSlots.erase(oldest);
            }
            mOrder.push_back(name);
        }
        mSlots[name] = frame;
    }

    bool FrameStore::get(const std::string& name, CapturedFrame& out) const{
        std::lock_guard<std::mutex> lock(mMutex);

        auto it = mSlots.find(name);
        if(it == mSlots.end()) return false;
        out = it->second;
        return true;
    }

    bool FrameStore::remove(const std::string& name){
        std::lock_guard<std::mutex> lock(mMutex);

        auto it = mSlots.find(name);
        if(it == mSlots.end()) return false;
        mSlots.erase(it);
        mOrder.erase(std::remove(mOrder.begin(), mOrder.end(), name), mOrder.end());
        return true;
    }

    void FrameStore::clear(){
        std::lock_guard<std::mutex> lock(mMutex);
        mSlots.clear();
        mOrder.clear();
    }

    std::vector<std::string> FrameStore::names() const{
        std::lock_guard<std::mutex> lock(mMutex);
        return std::vector<std::string>(mOrder.begin(), mOrder.end());
    }

    size_t FrameStore::size() const{
        std::lock_guard<std::mutex> lock(mMutex);
        return mSlots.size();
    }
}

#endif
