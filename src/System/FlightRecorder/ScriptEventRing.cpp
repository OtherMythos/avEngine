#ifdef FLIGHT_RECORDER

#include "ScriptEventRing.h"

#include <algorithm>

namespace AV{

    void ScriptEventRing::configure(uint32_t capacity){
        mCapacity = capacity;
        mEvents.assign(capacity, ScriptEvent());
        mNextIndex = 0;
        mStack.clear();
        mDeepest.clear();
    }

    void ScriptEventRing::pushCall(uint32_t funcIndex, int32_t line, uint64_t timeNs){
        if(mCapacity == 0) return;

        ScriptEvent& e = mEvents[mNextIndex % mCapacity];
        e.timeNs = timeNs;
        e.funcIndex = funcIndex;
        e.line = line;
        e.type = ScriptEventType::Call;
        mNextIndex++;

        mStack.push_back(funcIndex);
        if(mStack.size() > mDeepest.size()){
            mDeepest = mStack;
        }
    }

    void ScriptEventRing::pushReturn(int32_t line, uint64_t timeNs){
        if(mCapacity == 0) return;

        ScriptEvent& e = mEvents[mNextIndex % mCapacity];
        e.timeNs = timeNs;
        //A return carries no function of its own; the stack says which call it closes.
        e.funcIndex = mStack.empty() ? 0 : mStack.back();
        e.line = line;
        e.type = ScriptEventType::Return;
        mNextIndex++;

        if(!mStack.empty()) mStack.pop_back();
    }

    uint64_t ScriptEventRing::oldestIndex() const{
        if(mCapacity == 0) return mNextIndex;
        return mNextIndex > mCapacity ? mNextIndex - mCapacity : 0;
    }

    bool ScriptEventRing::available(uint64_t begin, uint64_t end) const{
        if(begin >= end) return begin == end;
        return begin >= oldestIndex() && end <= mNextIndex;
    }

    const ScriptEvent& ScriptEventRing::at(uint64_t absoluteIndex) const{
        return mEvents[absoluteIndex % mCapacity];
    }

    std::vector<ScriptEvent> ScriptEventRing::slice(uint64_t begin, uint64_t end) const{
        std::vector<ScriptEvent> out;
        if(mCapacity == 0) return out;

        const uint64_t from = std::max(begin, oldestIndex());
        const uint64_t to = std::min(end, mNextIndex);
        if(from >= to) return out;

        out.reserve(static_cast<size_t>(to - from));
        for(uint64_t i = from; i < to; i++){
            out.push_back(mEvents[i % mCapacity]);
        }
        return out;
    }

    std::vector<uint32_t> ScriptEventRing::takeDeepestStack(){
        std::vector<uint32_t> out = mDeepest;
        //The calls still open are genuinely still open, so they remain the new high water mark.
        mDeepest = mStack;
        return out;
    }

    size_t ScriptEventRing::size() const{
        return static_cast<size_t>(std::min<uint64_t>(mNextIndex, mCapacity));
    }

    void ScriptEventRing::clear(){
        mNextIndex = 0;
        mStack.clear();
        mDeepest.clear();
    }
}

#endif
