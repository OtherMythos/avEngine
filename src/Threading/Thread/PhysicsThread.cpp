#include "PhysicsThread.h"

#include "Logger/Log.h"

namespace AV{
    PhysicsThread::PhysicsThread()
        : mReady(false),
          mPhysicsManagerReady(false),
          mRunning(false) {
        
    }
    
    void PhysicsThread::run(){
        mRunning = true;
        
        std::unique_lock<std::mutex> runningLock(mRunningMutex);
        
        while(mRunning){
            if(!mReady || !mPhysicsManagerReady){
                //The world isn't ready, so we need to wait for it to become ready.
                AV_INFO("Waiting for world");
                //This will notify us when something happens to the world, and the checks should be re-performed.
                cv.wait(runningLock);
                continue;
            }
            AV_INFO("Updating Physics");
        }
        
    }
    
    void PhysicsThread::shutdown(){
        mRunning = false;
        cv.notify_all();
    }
    
    void PhysicsThread::setReady(bool ready){
        std::unique_lock<std::mutex>checkLock();
        mReady = ready;
        cv.notify_all();
    }
    
    void PhysicsThread::providePhysicsManager(std::shared_ptr<PhysicsManager> physicsManager){
        std::unique_lock<std::mutex>checkLock();
        
        mPhysicsManager = physicsManager;
        mPhysicsManagerReady = true;
    }
    
    void PhysicsThread::removePhysicsManager(){
        std::unique_lock<std::mutex>checkLock();
        
        //Clear the physics pointer. This might cause destruction.
        mPhysicsManager.reset();
        
        mPhysicsManagerReady = false;
    }
}
