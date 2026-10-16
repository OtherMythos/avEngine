#include "SimulationClock.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace AV{
    SimulationClock::SimulationClock(int rate) : mRate(rate){
        assert(rate > 0);
    }

    void SimulationClock::accumulate(double wallSeconds){
        if(mDiscardElapsed){
            mDiscardElapsed = false;
            return;
        }
        if(mMode != Mode::RUNNING || !std::isfinite(wallSeconds) || wallSeconds <= 0) return;
        mAccumulator += static_cast<long double>(std::min(wallSeconds, 0.25)) * mSpeed * mRate;
    }

    bool SimulationClock::beginTick(){
        assert(!mInTick);
        if(mMode == Mode::PAUSED) return false;
        if(mMode == Mode::RUNNING){
            if(mAccumulator < 1.0L - 1e-12L) return false;
            mAccumulator = std::max(0.0L, mAccumulator - 1.0L);
        }
        mInTick = true;
        return true;
    }

    void SimulationClock::finishTick(){
        assert(mInTick);
        ++mTick;
        mInTick = false;
        if(mMode == Mode::STEPPING){
            ++mStepExecuted;
            if(mStepExecuted == mStepRequested && !mPauseRequested){
                pause("step complete");
            }
        }
        if(mPauseRequested){
            mMode = Mode::PAUSED;
            mPauseRequested = false;
        }
    }

    void SimulationClock::pause(const std::string& reason){
        //Keep the first reason if several callbacks request a pause in the same tick.
        if(!mPauseRequested) mPauseReason = reason;
        mAccumulator = 0;
        mDiscardElapsed = true;
        if(mInTick) mPauseRequested = true;
        else mMode = Mode::PAUSED;
    }

    bool SimulationClock::resume(double speed){
        if(mInTick || mMode == Mode::STEPPING || !std::isfinite(speed) || speed <= 0 || speed > 1) return false;
        mSpeed = speed;
        mMode = Mode::RUNNING;
        mAccumulator = 0;
        mDiscardElapsed = true;
        return true;
    }

    bool SimulationClock::step(uint64_t ticks){
        if(mInTick || mMode != Mode::PAUSED || ticks == 0) return false;
        mMode = Mode::STEPPING;
        mStepStartTick = mTick;
        mStepRequested = ticks;
        mStepExecuted = 0;
        ++mStepId;
        mAccumulator = 0;
        mDiscardElapsed = true;
        return true;
    }
}
