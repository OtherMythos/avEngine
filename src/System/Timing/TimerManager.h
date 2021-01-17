#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/Util/VersionedDataPool.h"
#include "TimerManagerData.h"

namespace AV{

    class TimerManager{
    public:
        TimerManager();
        ~TimerManager();

        void update(TimeValue delta);

        TimerId addCountdownTimer(TimeValue timeout, SQObject closure, SQObject context);

        bool removeCountdownTimer(TimerId targetId);

    private:
        struct TimedEventEntry{
            TimeValue remainingTime;
            SQObject closure;
            SQObject context;
        };
        VersionedDataPool<TimedEventEntry> mDataPool;

        bool _internalRemoveCountdownTimer(TimerId targetId);

        void _callClosure(uint32 idx);
    };
}
