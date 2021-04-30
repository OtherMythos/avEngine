#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/Util/VersionedDataPool.h"
#include "TimerManagerData.h"

namespace AV{

    /**
    Manages the creation and lifetime of timer objects.
    Timer objects can register a squirrel callback and context to be called when an amount of time elapses.
    */
    class TimerManager{
    public:
        TimerManager();
        ~TimerManager();

        /**
        Update the manager with a time delta.
        */
        void update(TimeValue delta);

        /**
        Create a countdown timer object.
        @param timeout The number of milliseconds until the timer should call.
        @param closure The closure to call. Must be type OT_CLOSURE
        @param context A context to include as part of the closure call. If OT_NULL the context will be the root table.
        @returns A timer id to reference the timer.
        */
        TimerId addCountdownTimer(TimeValue timeout, SQObject closure, SQObject context);

        /**
        Remove a countdown timer based on timerId.
        @returns True or false depending on whether the timer could be removed succesfully.
        */
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
