#pragma once

#include <mutex>
#include <memory>
#include <atomic>
#include <condition_variable>

namespace AV{
    class PhysicsManager;
    class DynamicsWorldThreadLogic;
    
    class PhysicsThread{
    public:
        PhysicsThread();
        
        void run();
        void shutdown();
        
        void providePhysicsManager(std::shared_ptr<PhysicsManager> physicsManager);
        void removePhysicsManager();
        
        void setReady(bool ready);
        
    private:
        std::atomic<bool> mReady, mPhysicsManagerReady, mRunning, mWorldsShouldExist;
        std::condition_variable cv;
        
        std::mutex mReadyCheckMutex;
        std::mutex mRunningMutex;
        std::shared_ptr<PhysicsManager> mPhysicsManager;
        
        std::shared_ptr<DynamicsWorldThreadLogic> mDynLogic;
    };
}
