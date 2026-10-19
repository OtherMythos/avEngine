#pragma once

#include <cstdint>
#include <string>

namespace AV{
    //Main-thread owned. Wall time grants ticks; only completed ticks advance simulation time.
    class SimulationClock{
    public:
        enum class Mode{ RUNNING, PAUSED, STEPPING };
        explicit SimulationClock(int rate = 60);

        void accumulate(double wallSeconds);
        bool beginTick();
        void finishTick();
        void pause(const std::string& reason);
        bool resume(double speed);
        bool step(uint64_t ticks);
        void setOfflineCapture(bool enabled);

        Mode mode() const { return mMode; }
        bool inTick() const { return mInTick; }
        bool offlineCapture() const { return mOfflineCapture; }
        int rate() const { return mRate; }
        double deltaSeconds() const { return 1.0 / mRate; }
        double seconds() const { return static_cast<double>(mTick) / mRate; }
        uint64_t tick() const { return mTick; }
        double speed() const { return mSpeed; }
        const std::string& pauseReason() const { return mPauseReason; }
        uint64_t stepId() const { return mStepId; }
        uint64_t stepStartTick() const { return mStepStartTick; }
        uint64_t stepRequested() const { return mStepRequested; }
        uint64_t stepExecuted() const { return mStepExecuted; }

    private:
        int mRate;
        Mode mMode = Mode::RUNNING;
        uint64_t mTick = 0;
        long double mAccumulator = 0;
        double mSpeed = 1.0;
        bool mInTick = false;
        bool mPauseRequested = false;
        bool mDiscardElapsed = true;
        bool mOfflineCapture = false;
        std::string mPauseReason;
        uint64_t mStepId = 0, mStepStartTick = 0, mStepRequested = 0, mStepExecuted = 0;
    };
}
