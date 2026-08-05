#pragma once

#include <thread>
#include <memory>

namespace AV{
    class PhysicsBodyDestructor;
    class PhysicsThread;
    class PhysicsManager;

    /**
    A class to manage devoted threads in the engine.

    For instance, physics threads, ai threads.
    This is for threads which are incompatible with the architecture of the job system,
    in that the calculations they perform cannot wait to be performed.
    Physics needs to be updated each frame, so it is given its own devoted thread.
    Loading a resource from disk does not require this level of urgency, so it can be performed as a job.
    */
    class ThreadManager{
    public:
        ThreadManager();
        ~ThreadManager();

        void initialise();

        void notifyPhysicsManagerCreated(std::shared_ptr<PhysicsManager> physicsManager);
        void notifyPhysicsManagerDestroyed();

        /**
        Hand the physics thread one fixed update's worth of simulation time. Returns immediately;
        pair every call with a waitForPhysicsStep on the next update.
        */
        void schedulePhysicsStep(double deltaSeconds);
        /**
        Block until the previously scheduled physics step has completed and its results are safe
        to read. A no-op if nothing is outstanding.
        */
        void waitForPhysicsStep();




    private:
        std::thread* mPhysicsThread;

        PhysicsThread* mPhysicsThreadInstance;
    };
}
