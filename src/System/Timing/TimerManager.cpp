#include "TimerManager.h"

#include "Scripting/ScriptVM.h"

namespace AV{
    TimerManager::TimerManager(){

    }

    TimerManager::~TimerManager(){

    }

    void TimerManager::update(TimeValue delta){
        if(delta == 0) return;
        const std::vector<DataPool<TimedEventEntry>::DataEntry>& data = mDataPool.getInternalData();
        for(uint32 i = 0; i < data.size(); i++){
            const DataPool<TimedEventEntry>::DataEntry& e = data[i];
            if(e.first.remainingTime == 0) continue;
            if(e.first.remainingTime <= delta){
                //Produce the id with the correct version.
                TimerId targetId = i & (static_cast<uint64>(mDataPool.getVersions()[i]) << 32);
                _callClosure(i);
                _internalRemoveCountdownTimer(i);
                continue;
            }
            mDataPool.getEntry(i).remainingTime -= delta;
        }
    }

    TimerId TimerManager::addCountdownTimer(TimeValue timeout, SQObject closure, SQObject context){
        assert(closure._type == OT_CLOSURE);

        ScriptVM::referenceObject(closure);
        ScriptVM::referenceObject(context);

        return mDataPool.storeEntry({timeout, closure, context});
    }

    bool TimerManager::removeCountdownTimer(TimerId targetId){
        return _internalRemoveCountdownTimer(targetId);
    }

    bool TimerManager::_internalRemoveCountdownTimer(TimerId targetId){
        TimedEventEntry& entry = mDataPool.getEntry(targetId);

        ScriptVM::dereferenceObject(entry.closure);
        ScriptVM::dereferenceObject(entry.context);
        entry.remainingTime = 0;
        mDataPool.removeEntry(targetId);

        return true;
    }

    void TimerManager::_callClosure(uint32 idx){
        const TimedEventEntry& e = mDataPool.getEntry(idx);
        ScriptVM::callClosure(e.closure, &(e.context));
    }
}
