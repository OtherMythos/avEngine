#pragma once

#include <thread>

namespace AV{
    class PhysicsBodyDestructor;
    class PhysicsThread;
    class Event;

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

        void sheduleUpdate(int time);

        bool worldEventReceiver(const Event &e);



    private:
        std::thread* mPhysicsThread;

        PhysicsThread* mPhysicsThreadInstance;
    };
}
