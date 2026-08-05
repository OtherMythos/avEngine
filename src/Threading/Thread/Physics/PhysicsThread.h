#pragma once

#include <mutex>
#include <memory>
#include <atomic>
#include <condition_variable>
#include <vector>

#include "System/EnginePrerequisites.h"

namespace AV{
    class PhysicsManager;
    class PhysicsWorldThreadLogic;
    class DynamicsWorldThreadLogic;
    class CollisionWorldThreadLogic;

    /**
    Runs the physics simulation on its own thread, in lockstep with the engine's fixed update.

    ================================ THE HANDSHAKE ================================
    The main thread and this thread agree on a step at a time:

        main:     waitForScheduledStep()   <- blocks until the previous step finished
                  ...drain results, run game logic, queue new commands...
                  scheduleStep(delta)      <- hands over the fixed delta, returns immediately
        physics:  wakes, runs however many steps of the configured timestep that delta buys,
                  then acknowledges

    So exactly one round of physics happens per fixed update, and a command queued by game logic
    during update U is processed by the step scheduled at the end of U and visible in U+1.
    Physics runs concurrently with the rest of the update and with rendering, so the wait is
    normally free; it only costs anything when a step outlasts everything else in the frame.

    mScheduledStep and mCompletedStep are monotonic counters rather than a flag, so waiting when
    nothing was ever scheduled is correctly a no-op, and a hung handshake can be diagnosed by
    reading the two numbers.

    THE INVARIANT THAT MATTERS: a scheduled step is ALWAYS acknowledged, even when physics is
    paused, no world exists, or the engine is shutting down. The ready and pause checks decide
    whether the worlds are stepped, never whether we acknowledge. If an acknowledgement is ever
    skipped the main thread blocks forever, so the acknowledgement is done by an RAII object
    rather than by a statement someone could branch around.
    ===============================================================================
    */
    class PhysicsThread{
    public:
        PhysicsThread();
        /**
        Construct with explicit world logic, for tests. Skips the PhysicsBodyDestructor
        registration the normal constructor performs.
        */
        PhysicsThread(std::shared_ptr<PhysicsWorldThreadLogic> dynLogic, std::vector<std::shared_ptr<PhysicsWorldThreadLogic>> collisionWorlds);

        void run();
        void shutdown();

        void notifyPhysicsManagerCreated(std::shared_ptr<PhysicsManager> physicsManager);
        void notifyPhysicsManagerDestroyed();

        /**
        Hand the physics thread a slice of simulation time and return immediately.
        Called by the main thread once per fixed update.
        */
        void scheduleStep(int64 deltaNs);

        /**
        Block until the most recently scheduled step has been acknowledged.
        Returns immediately if nothing is outstanding, or if the thread is shutting down.
        */
        void waitForScheduledStep();

        void setReady(bool ready);

    protected:
        //Read from other threads, so still atomic. Every write is followed by a notify issued
        //under mStateMutex, which is what stops a wakeup being lost.
        std::atomic<bool> mReady, mPhysicsManagerReady, mWorldsShouldExist;
        std::atomic<int> mCurrentWorldVersion;

        //Guards everything below it, and is the lock both condition variables wait on.
        std::mutex mStateMutex;
        //Plain bool rather than atomic: the wait predicate has to be evaluated under the same
        //lock the notifier holds, which atomicity alone would not give.
        bool mRunning = false;

        //The main thread waits here for a step to be acknowledged. Protected alongside the two
        //counters so a test can wait on the real condition variable with the real predicate,
        //rather than polling - polling a handshake that completes in microseconds costs a
        //millisecond a step and made these tests orders of magnitude slower than the work.
        std::condition_variable mDoneCV;
        //Incremented by the main thread, assigned by the physics thread once serviced.
        uint64 mScheduledStep = 0;
        uint64 mCompletedStep = 0;

    private:
        std::shared_ptr<PhysicsWorldThreadLogic> mDynLogic;

        uint8 mActiveCollisionWorlds;
        std::shared_ptr<PhysicsWorldThreadLogic> mCollisionWorlds[MAX_COLLISION_WORLDS];

        //The physics thread waits here for work.
        std::condition_variable mWorkCV;

        int64 mPendingDeltaNs = 0;
        //Set when something happened that needs a world construct/destruct pass with no step.
        bool mWorldStateDirty = false;

        //Physics thread only.
        int64 mAccumulatorNs = 0;
        //Nanoseconds rather than a double, because a double accumulator drifts for ratios like
        //50/60 and a drifting accumulator produces an occasional extra or missing step.
        int64 mTimestepNs = 0;
        int mMaxStepsPerUpdate = 0;

        void _initialiseCollisionWorldSlots();
        //Read from SystemSettings on first use rather than in the constructor, which is not
        //guaranteed to run after the setup file has been parsed.
        void _initTimestepIfNeeded();
        void _stepWorlds(float timeStep, uint32 pauseMask);

        /**
        Acknowledges a scheduled step on destruction, whatever path the body exits by.
        */
        class StepAcknowledger{
        public:
            StepAcknowledger(PhysicsThread* parent, bool haveStep, uint64 target)
                : mParent(parent), mHaveStep(haveStep), mTarget(target) {}
            ~StepAcknowledger();
        private:
            PhysicsThread* mParent;
            bool mHaveStep;
            uint64 mTarget;
        };
        friend class StepAcknowledger;
    };
}
