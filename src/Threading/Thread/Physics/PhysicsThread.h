#pragma once

#include <mutex>
#include <memory>
#include <atomic>
#include <condition_variable>

#include "System/EnginePrerequisites.h"

namespace AV{
    class PhysicsManager;
    class DynamicsWorldThreadLogic;
    class CollisionWorldThreadLogic;

    class PhysicsThread{
    public:
        PhysicsThread();

        void run();
        void shutdown();

        void notifyWorldCreation(std::shared_ptr<PhysicsManager> physicsManager);
        void notifyWorldDestruction();

        void scheduleWorldUpdate(int time);

        void setReady(bool ready);

    private:
        std::atomic<bool> mReady, mPhysicsManagerReady, mRunning, mWorldsShouldExist;
        std::atomic<int> mTimestepSync, mCurrentWorldVersion;
        std::condition_variable cv;

        std::mutex mReadyCheckMutex;
        std::mutex mRunningMutex;

        void _determineCreationDestruction();

        std::shared_ptr<DynamicsWorldThreadLogic> mDynLogic;

        uint8 mActiveCollisionWorlds;
        std::shared_ptr<CollisionWorldThreadLogic> mCollisionWorlds[MAX_COLLISION_WORLDS];
    };
}
