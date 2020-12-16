#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/Util/DataPool.h"
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
        DataPool<TimedEventEntry> mDataPool;
        std::vector<uint32> mTimerVersions;

        bool _internalRemoveCountdownTimer(TimerId targetId);
        bool _isTimerValid(TimerId id) const;
        inline bool _internalIsTimerValid(uint32 id, uint32 version) const;

        void _callClosure(uint32 idx);

        inline void _getValues(TimerId id, uint32* outId, uint32* outVersion) const;
    };
}
