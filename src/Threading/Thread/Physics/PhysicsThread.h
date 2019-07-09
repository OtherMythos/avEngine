#pragma once

#include <mutex>
#include <memory>
#include <atomic>
#include <condition_variable>

namespace AV{
    class PhysicsManager;
    class DynamicsWorldThreadLogic;
    class PhysicsBodyDestructor;

    class PhysicsThread{
    public:
        PhysicsThread(std::shared_ptr<PhysicsBodyDestructor> destructor);

        void run();
        void shutdown();

        void providePhysicsManager(std::shared_ptr<PhysicsManager> physicsManager);
        void removePhysicsManager();

        void scheduleWorldUpdate(int time);

        void setReady(bool ready);

    private:
        std::atomic<bool> mReady, mPhysicsManagerReady, mRunning, mWorldsShouldExist;
        std::atomic<int> mTimestepSync;
        std::condition_variable cv;

        std::mutex mReadyCheckMutex;
        std::mutex mRunningMutex;

        std::shared_ptr<DynamicsWorldThreadLogic> mDynLogic;
    };
}
