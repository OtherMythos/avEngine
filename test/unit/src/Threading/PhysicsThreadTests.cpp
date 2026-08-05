#include "gtest/gtest.h"

#include <atomic>
#include <cmath>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "Threading/Thread/Physics/PhysicsThread.h"
#include "Threading/Thread/Physics/PhysicsWorldThreadLogic.h"
#include "System/Pause/PauseState.h"
#include "unit/src/TestAccessors.h"

using namespace AV;
using namespace std::chrono;

/*
Covers the handshake between the main thread and the physics thread.

The failure mode this file exists for is a HANG: if a scheduled step is ever not acknowledged, the
main thread blocks in waitForScheduledStep forever. So every wait here is bounded, and a bound that
elapses fails the test rather than wedging the run.
*/

namespace {

    //Counts calls and touches no bullet API. checkWorldConstructDestruct on the base class only
    //calls the two pure virtuals below, so this is all a fake needs.
    class FakeWorldLogic : public PhysicsWorldThreadLogic{
    public:
        std::atomic<int> updateCount{0};
        std::atomic<int> constructCount{0};
        std::atomic<int> destroyCount{0};
        std::atomic<float> lastTimeStep{0.0f};
        //Records the order calls arrive in across all fakes, to prove lockstep.
        static std::atomic<int> sSequence;
        std::atomic<int> lastSequence{-1};

        void updateWorld(float timeStep) override{
            lastTimeStep = timeStep;
            lastSequence = sSequence++;
            updateCount++;
        }
    protected:
        void constructWorld() override { constructCount++; mPhysicsWorld = reinterpret_cast<btCollisionWorld*>(1); }
        void destroyWorld() override { destroyCount++; mPhysicsWorld = 0; mWorldDestroyComplete = true; }
    };
    std::atomic<int> FakeWorldLogic::sSequence{0};

    //Re-exposes the protected state the tests drive, the same way TestAccessors.h does for the
    //static singletons. Kept here rather than in that header because PhysicsThread is instantiated
    //rather than a singleton, so this is the only file that can use it.
    class TestablePhysicsThread : public PhysicsThread{
    public:
        using PhysicsThread::PhysicsThread;

        using PhysicsThread::mPhysicsManagerReady;
        using PhysicsThread::mWorldsShouldExist;
        using PhysicsThread::mCurrentWorldVersion;
        using PhysicsThread::mStateMutex;
        using PhysicsThread::mRunning;
    };

    class PhysicsThreadFixture : public ::testing::Test{
    protected:
        void SetUp() override{
            mPreviousMask = PauseState::getMask();
            mPreviousRate = TestableSystemSettings::mPhysicsUpdateRate;
            mPreviousMaxSteps = TestableSystemSettings::mMaxPhysicsStepsPerUpdate;
            PauseState::setMask(0);
            FakeWorldLogic::sSequence = 0;
        }

        void TearDown() override{
            stopThread();
            PauseState::setMask(mPreviousMask);
            TestableSystemSettings::mPhysicsUpdateRate = mPreviousRate;
            TestableSystemSettings::mMaxPhysicsStepsPerUpdate = mPreviousMaxSteps;
        }

        //Rates have to be set before the thread services its first step, since the timestep is
        //read lazily once.
        void startThread(int collisionWorldCount = 1, bool withDynamics = true, bool ready = true){
            if(withDynamics) mDyn = std::make_shared<FakeWorldLogic>();
            std::vector<std::shared_ptr<PhysicsWorldThreadLogic>> collision;
            for(int i = 0; i < collisionWorldCount; i++){
                mCollision.push_back(std::make_shared<FakeWorldLogic>());
                collision.push_back(mCollision.back());
            }

            mThread.reset(new TestablePhysicsThread(mDyn, collision));
            if(ready){
                mThread->mPhysicsManagerReady = true;
                mThread->setReady(true);
            }
            mRunner.reset(new std::thread(&PhysicsThread::run, mThread.get()));

            //The wait predicate includes !mRunning, so a wait issued before run() starts would
            //return early. Settle first so the tests measure what they mean to.
            const auto deadline = steady_clock::now() + seconds(2);
            while(steady_clock::now() < deadline){
                std::lock_guard<std::mutex> lock(mThread->mStateMutex);
                if(mThread->mRunning) break;
            }
        }

        void stopThread(){
            if(!mRunner) return;
            mThread->shutdown();
            mRunner->join();
            mRunner.reset();
            mThread.reset();
        }

        /**
        Wait for the outstanding step, giving up after a bound. On timeout the physics thread is
        shut down so the blocked waiter is released and the test can finish rather than hang.
        */
        bool boundedWait(int timeoutMs = 3000){
            std::atomic<bool> done{false};
            std::thread waiter([&]{ mThread->waitForScheduledStep(); done = true; });

            const auto deadline = steady_clock::now() + milliseconds(timeoutMs);
            while(!done.load() && steady_clock::now() < deadline){
                std::this_thread::sleep_for(milliseconds(1));
            }

            const bool completed = done.load();
            if(!completed) mThread->shutdown();
            waiter.join();

            return completed;
        }

        //One fixed update's worth of the handshake. Rounds exactly as
        //ThreadManager::schedulePhysicsStep does, so these exercise the real conversion.
        bool stepOnce(double deltaSeconds){
            mThread->scheduleStep(llround(deltaSeconds * 1000000000.0));
            return boundedWait();
        }

        int dynCount() const { return mDyn ? mDyn->updateCount.load() : 0; }
        int collisionCount(size_t i) const { return mCollision[i]->updateCount.load(); }

        std::shared_ptr<FakeWorldLogic> mDyn;
        std::vector<std::shared_ptr<FakeWorldLogic>> mCollision;
        std::unique_ptr<TestablePhysicsThread> mThread;
        std::unique_ptr<std::thread> mRunner;

        PauseMask mPreviousMask = 0;
        int mPreviousRate = 60;
        int mPreviousMaxSteps = 4;
    };
}

TEST_F(PhysicsThreadFixture, waitingWithNothingScheduledReturnsImmediately){
    startThread();

    //The very first update of the engine's life does this, before anything has been scheduled.
    ASSERT_TRUE(boundedWait(1000));
    ASSERT_EQ(dynCount(), 0);
}

TEST_F(PhysicsThreadFixture, oneScheduledStepRunsExactlyOneStep){
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    startThread();

    ASSERT_TRUE(stepOnce(1.0 / 60.0));

    ASSERT_EQ(dynCount(), 1);
    ASSERT_EQ(collisionCount(0), 1);
    ASSERT_NEAR(mDyn->lastTimeStep.load(), 1.0f / 60.0f, 0.0001f);
}

TEST_F(PhysicsThreadFixture, aFasterPhysicsRateRunsProportionallyMoreSteps){
    //30Hz game logic feeding a 60Hz simulation is exactly two steps every update.
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    startThread();

    ASSERT_TRUE(stepOnce(1.0 / 30.0));

    ASSERT_EQ(dynCount(), 2);
    ASSERT_EQ(collisionCount(0), 2);
}

TEST_F(PhysicsThreadFixture, aSlowerPhysicsRateRunsAStepEveryOtherUpdate){
    TestableSystemSettings::mPhysicsUpdateRate = 30;
    startThread();

    ASSERT_TRUE(stepOnce(1.0 / 60.0));
    ASSERT_EQ(dynCount(), 0);   //not enough time banked yet
    ASSERT_TRUE(stepOnce(1.0 / 60.0));
    ASSERT_EQ(dynCount(), 1);
}

TEST_F(PhysicsThreadFixture, theAccumulatorDoesNotDriftOverManyUpdates){
    //The guarantee the whole design rests on: a constant delta in means a constant step count out,
    //forever. A double accumulator drifts here for rates which do not divide evenly.
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    startThread();

    const int updates = 1000;
    for(int i = 0; i < updates; i++){
        ASSERT_TRUE(stepOnce(1.0 / 30.0)) << "handshake stalled on update " << i;
    }

    ASSERT_EQ(dynCount(), updates * 2);
    ASSERT_EQ(collisionCount(0), updates * 2);
}

TEST_F(PhysicsThreadFixture, theAccumulatorDoesNotDriftForRatesWhichDoNotDivideEvenly){
    //50Hz logic into a 60Hz simulation. Not a whole ratio, so the step count per update alternates
    //- but the total must still be exact, which is what integer nanoseconds buy.
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    startThread();

    const int updates = 600;
    for(int i = 0; i < updates; i++){
        ASSERT_TRUE(stepOnce(1.0 / 50.0)) << "handshake stalled on update " << i;
    }

    //600 updates of 1/50s is 12 seconds of simulated time, which at 60Hz is 720 steps.
    //Allow a single step of slack for the truncation in the nanosecond timestep.
    ASSERT_NEAR(dynCount(), 720, 1);
}

TEST_F(PhysicsThreadFixture, stepsPerUpdateAreCapped){
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    TestableSystemSettings::mMaxPhysicsStepsPerUpdate = 4;
    startThread();

    //A whole second of delta would be 60 steps without the cap.
    ASSERT_TRUE(stepOnce(1.0));
    ASSERT_EQ(dynCount(), 4);

    //And the surplus is dropped rather than banked, so the next update is normal.
    ASSERT_TRUE(stepOnce(1.0 / 60.0));
    ASSERT_EQ(dynCount(), 5);
}

TEST_F(PhysicsThreadFixture, aScheduledStepIsAcknowledgedWhilePaused){
    //THE deadlock regression test. Before the handshake existed, pause parked the thread in a wait
    //and it never acknowledged - which under a blocking main thread is a hang.
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    startThread();

    PauseState::setMask(PAUSE_TYPE_PHYSICS);

    ASSERT_TRUE(stepOnce(1.0 / 60.0)) << "a paused physics thread failed to acknowledge its step";
    ASSERT_EQ(dynCount(), 0);
    ASSERT_EQ(collisionCount(0), 0);
}

TEST_F(PhysicsThreadFixture, aScheduledStepIsAcknowledgedWhenNotReady){
    startThread(1, true, false);
    mThread->mPhysicsManagerReady = true;
    mThread->setReady(false);

    ASSERT_TRUE(stepOnce(1.0 / 60.0)) << "an unready physics thread failed to acknowledge its step";
    ASSERT_EQ(dynCount(), 0);
}

TEST_F(PhysicsThreadFixture, aScheduledStepIsAcknowledgedWithNoPhysicsManager){
    //The normal state at engine startup, before any world has been created.
    startThread(1, true, false);
    mThread->setReady(true);

    ASSERT_TRUE(stepOnce(1.0 / 60.0)) << "a physics thread with no world failed to acknowledge its step";
    ASSERT_EQ(dynCount(), 0);
}

TEST_F(PhysicsThreadFixture, timeIsNotBankedAcrossAPause){
    //Otherwise unpausing would fire a burst of steps and every collision count would jump.
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    startThread();

    PauseState::setMask(PAUSE_TYPE_PHYSICS);
    for(int i = 0; i < 100; i++) ASSERT_TRUE(stepOnce(1.0 / 60.0));
    ASSERT_EQ(dynCount(), 0);

    PauseState::setMask(0);
    ASSERT_TRUE(stepOnce(1.0 / 60.0));
    ASSERT_EQ(dynCount(), 1) << "unpausing released banked time as a burst";
}

TEST_F(PhysicsThreadFixture, perWorldPauseGatingIsPreserved){
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    startThread(3);

    PauseState::setMask(PAUSE_TYPE_PHYSICS_DYNAMICS);
    ASSERT_TRUE(stepOnce(1.0 / 60.0));
    ASSERT_EQ(dynCount(), 0);
    ASSERT_EQ(collisionCount(0), 1);
    ASSERT_EQ(collisionCount(2), 1);

    //Just collision world 1.
    PauseState::setMask(PAUSE_TYPE_PHYSICS_COLLISION0 << 1);
    ASSERT_TRUE(stepOnce(1.0 / 60.0));
    ASSERT_EQ(dynCount(), 1);
    ASSERT_EQ(collisionCount(0), 2);
    ASSERT_EQ(collisionCount(1), 1);
    ASSERT_EQ(collisionCount(2), 2);

    //Every collision world.
    PauseState::setMask(PAUSE_TYPE_PHYSICS_COLLISION);
    ASSERT_TRUE(stepOnce(1.0 / 60.0));
    ASSERT_EQ(dynCount(), 2);
    ASSERT_EQ(collisionCount(0), 2);
    ASSERT_EQ(collisionCount(2), 2);
}

TEST_F(PhysicsThreadFixture, collisionStepsInLockstepWithDynamics){
    //Two steps per update must be dynamics,collision,dynamics,collision - not dynamics twice then
    //collision twice - or collision event counts stop matching the simulation.
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    startThread(1);

    ASSERT_TRUE(stepOnce(1.0 / 30.0));

    ASSERT_EQ(dynCount(), 2);
    ASSERT_EQ(collisionCount(0), 2);
    //The last collision call must be the most recent of all four.
    ASSERT_GT(mCollision[0]->lastSequence.load(), mDyn->lastSequence.load());
    ASSERT_EQ(mCollision[0]->lastSequence.load(), 3);
}

TEST_F(PhysicsThreadFixture, shutdownReleasesABlockedWaiter){
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    //Never becomes ready, so nothing will service the step; only shutdown can release the waiter.
    startThread(1, true, false);

    std::atomic<bool> released{false};
    mThread->scheduleStep(16666666);
    //Hold the thread off servicing by pausing it, then block a waiter behind it.
    std::thread waiter([&]{ mThread->waitForScheduledStep(); released = true; });

    std::this_thread::sleep_for(milliseconds(20));
    mThread->shutdown();

    const auto deadline = steady_clock::now() + seconds(3);
    while(!released.load() && steady_clock::now() < deadline) std::this_thread::sleep_for(milliseconds(1));

    ASSERT_TRUE(released.load()) << "shutdown left a waiter blocked";
    waiter.join();

    mRunner->join();
    mRunner.reset();
    mThread.reset();
}

TEST_F(PhysicsThreadFixture, noWakeupsAreLostUnderTightScheduling){
    //The old scheduleWorldUpdate notified without holding the mutex, so a notify could land between
    //the predicate check and the sleep and be lost entirely.
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    startThread();

    const int updates = 5000;
    for(int i = 0; i < updates; i++){
        ASSERT_TRUE(stepOnce(1.0 / 60.0)) << "wakeup lost on update " << i;
    }

    ASSERT_EQ(dynCount(), updates);
}

TEST_F(PhysicsThreadFixture, worldConstructionStillHappens){
    TestableSystemSettings::mPhysicsUpdateRate = 60;
    startThread(1, true, false);

    //What notifyWorldCreation does, without needing a real PhysicsManager.
    mThread->mPhysicsManagerReady = true;
    mThread->mWorldsShouldExist = true;
    mThread->mCurrentWorldVersion++;
    mThread->setReady(true);

    ASSERT_TRUE(stepOnce(1.0 / 60.0));
    ASSERT_EQ(mDyn->constructCount.load(), 1);
    ASSERT_EQ(mCollision[0]->constructCount.load(), 1);

    //And destruction, which the thread must notice even with no step scheduled.
    mThread->mWorldsShouldExist = false;
    mThread->setReady(false);

    const auto deadline = steady_clock::now() + seconds(3);
    while(mDyn->destroyCount.load() == 0 && steady_clock::now() < deadline){
        std::this_thread::sleep_for(milliseconds(1));
    }
    ASSERT_EQ(mDyn->destroyCount.load(), 1);
}
