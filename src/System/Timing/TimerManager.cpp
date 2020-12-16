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
                TimerId targetId = i & (static_cast<uint64>(mTimerVersions[i]) << 32);
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

        TimerId outVal = 0;
        uint32 id = mDataPool.storeEntry({timeout, closure, context});

        outVal &= id;

        uint32 foundVersion = 0;
        if(mTimerVersions.size() < id){
            foundVersion = mTimerVersions[id];
        }else{
            assert(id == mTimerVersions.size());
            mTimerVersions.push_back(0);
        }
        uint64 castVersion = static_cast<uint64>(foundVersion);
        outVal &= (castVersion << 32);

        return outVal;
    }

    bool TimerManager::removeCountdownTimer(TimerId targetId){
        return _internalRemoveCountdownTimer(targetId);
    }

    bool TimerManager::_internalRemoveCountdownTimer(TimerId targetId){
        uint32 foundId, foundVersion;
        _getValues(targetId, &foundId, &foundVersion);
        if(!_internalIsTimerValid(foundId, foundVersion)) return false;

        TimedEventEntry& entry = mDataPool.getEntry(foundId);

        ScriptVM::dereferenceObject(entry.closure);
        ScriptVM::dereferenceObject(entry.context);
        entry.remainingTime = 0;
        mDataPool.removeEntry(foundId);

        return true;
    }

    void TimerManager::_callClosure(uint32 idx){
        const TimedEventEntry& e = mDataPool.getEntry(idx);
        ScriptVM::callClosure(e.closure, &(e.context));
    }

    bool TimerManager::_isTimerValid(TimerId id) const{
        uint32 foundId, foundVersion;
        _getValues(id, &foundId, &foundVersion);

        assert(foundId < mDataPool.getInternalData().size());
        assert(foundId < mTimerVersions.size());

        return _internalIsTimerValid(foundId, foundVersion);
    }

    bool TimerManager::_internalIsTimerValid(uint32 id, uint32 version) const{
        return version == mTimerVersions[id];
    }

    void TimerManager::_getValues(TimerId id, uint32* outId, uint32* outVersion) const{
        *outVersion = static_cast<uint32>((id >> 32) & 0xffffffff);
        *outId = static_cast<uint32>(id);
    }
}
