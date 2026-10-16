#include "gtest/gtest.h"
#include "System/Simulation/SimulationClock.h"
#include <limits>

using namespace AV;

namespace{
    unsigned drain(SimulationClock& clock){
        unsigned count = 0;
        while(clock.beginTick()){
            clock.finishTick();
            ++count;
        }
        return count;
    }
}

TEST(SimulationClockTests, SlowdownKeepsTheFixedDelta){
    SimulationClock clock(60);
    ASSERT_TRUE(clock.resume(0.25));
    clock.accumulate(0);
    clock.accumulate(0.2);
    EXPECT_EQ(drain(clock), 3u);
    EXPECT_DOUBLE_EQ(clock.deltaSeconds(), 1.0 / 60);
    EXPECT_DOUBLE_EQ(clock.seconds(), 3.0 / 60);
}

TEST(SimulationClockTests, PausedWallTimeAndOldDebtAreDiscarded){
    SimulationClock clock;
    clock.accumulate(0);
    clock.accumulate(0.2);
    clock.pause("inspect");
    clock.accumulate(3600);
    EXPECT_EQ(drain(clock), 0u);
    ASSERT_TRUE(clock.resume(1));
    clock.accumulate(3600);
    EXPECT_EQ(drain(clock), 0u);
    clock.accumulate(1.0 / 60);
    EXPECT_EQ(drain(clock), 1u);
}

TEST(SimulationClockTests, ExactStepsIgnoreWallTimeAndFinishPaused){
    SimulationClock clock(120);
    EXPECT_FALSE(clock.step(10));
    clock.pause("ready");
    ASSERT_TRUE(clock.step(10));
    clock.accumulate(1000);
    EXPECT_EQ(drain(clock), 10u);
    EXPECT_EQ(clock.mode(), SimulationClock::Mode::PAUSED);
    EXPECT_EQ(clock.stepExecuted(), 10u);
    EXPECT_DOUBLE_EQ(clock.seconds(), 10.0 / 120);
    EXPECT_EQ(clock.pauseReason(), "step complete");
}

TEST(SimulationClockTests, ScriptPauseFinishesCurrentTickAndCancelsRemainingSteps){
    SimulationClock clock;
    clock.pause("ready");
    ASSERT_TRUE(clock.step(20));
    ASSERT_TRUE(clock.beginTick());
    clock.pause("health decreased");
    clock.pause("later event");
    EXPECT_EQ(clock.tick(), 0u);
    clock.finishTick();
    EXPECT_EQ(clock.tick(), 1u);
    EXPECT_EQ(clock.stepExecuted(), 1u);
    EXPECT_FALSE(clock.beginTick());
    EXPECT_EQ(clock.pauseReason(), "health decreased");
    ASSERT_TRUE(clock.step(2));
    EXPECT_EQ(drain(clock), 2u);
    EXPECT_EQ(clock.tick(), 3u);
    EXPECT_EQ(clock.stepId(), 2u);
    EXPECT_EQ(clock.stepStartTick(), 1u);
}

TEST(SimulationClockTests, ScriptReasonWinsOnFinalRequestedTick){
    SimulationClock clock;
    clock.pause("ready");
    ASSERT_TRUE(clock.step(1));
    ASSERT_TRUE(clock.beginTick());
    clock.pause("damage");
    clock.finishTick();
    EXPECT_EQ(clock.pauseReason(), "damage");
    EXPECT_EQ(clock.mode(), SimulationClock::Mode::PAUSED);
}

TEST(SimulationClockTests, PauseInterruptsRealtimeCatchup){
    SimulationClock clock;
    clock.accumulate(0);
    clock.accumulate(0.2);
    ASSERT_TRUE(clock.beginTick());
    clock.pause("damage");
    clock.finishTick();
    EXPECT_EQ(drain(clock), 0u);
    EXPECT_EQ(clock.tick(), 1u);
}

TEST(SimulationClockTests, RejectsConflictingAndInvalidControls){
    SimulationClock clock;
    EXPECT_FALSE(clock.resume(0));
    EXPECT_FALSE(clock.resume(-1));
    EXPECT_FALSE(clock.resume(2));
    EXPECT_FALSE(clock.resume(std::numeric_limits<double>::quiet_NaN()));
    clock.pause("ready");
    EXPECT_FALSE(clock.step(0));
    ASSERT_TRUE(clock.step(2));
    EXPECT_FALSE(clock.step(2));
    EXPECT_FALSE(clock.resume(1));
    EXPECT_EQ(drain(clock), 2u);
}

TEST(SimulationClockTests, TickTimeDoesNotAccumulateFloatingPointDrift){
    SimulationClock clock(144);
    clock.pause("ready");
    ASSERT_TRUE(clock.step(14400));
    EXPECT_EQ(drain(clock), 14400u);
    EXPECT_DOUBLE_EQ(clock.seconds(), 100.0);
}

TEST(SimulationClockTests, WallTimePartitionDoesNotChangeTickCount){
    SimulationClock clock(120);
    clock.accumulate(0);
    unsigned count = 0;
    for(int i = 0; i < 1000; ++i){
        clock.accumulate(0.001);
        count += drain(clock);
    }
    EXPECT_EQ(count, 120u);
}
