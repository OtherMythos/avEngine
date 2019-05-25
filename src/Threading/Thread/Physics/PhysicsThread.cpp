#include "PhysicsThread.h"

#include "Logger/Log.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"
#include "DynamicsWorldThreadLogic.h"

#include "btBulletDynamicsCommon.h"

namespace AV{
    PhysicsThread::PhysicsThread()
        : mReady(false),
          mPhysicsManagerReady(false),
          mRunning(false),
          mWorldsShouldExist(false),
          mDynLogic(std::shared_ptr<DynamicsWorldThreadLogic>(new DynamicsWorldThreadLogic())) {
        
    }
    
    void PhysicsThread::run(){
        mRunning = true;
        
        std::unique_lock<std::mutex> runningLock(mRunningMutex);
        
        while(mRunning){
            //Check if the world needs destruction.
            mDynLogic->checkWorldConstructDestruct(mWorldsShouldExist);
            
            if(!mReady || !mPhysicsManagerReady){
                //The world isn't ready, so we need to wait for it to become ready.
                AV_INFO("Waiting for world");
                //This will notify us when something happens to the world, and the checks should be re-performed.
                cv.wait(runningLock);
                continue;
            }
            mDynLogic->updateWorld();
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
        //The world is created
        std::unique_lock<std::mutex>checkLock();
        
        mPhysicsManager = physicsManager;
        mPhysicsManagerReady = true;
        
        //This function is called by the main thread.
        //Here the world would need to be flagged as created, but this cannot be performed by the main thread.
        //So a flag is set to tell the thread to create the world when an update tick happens.
        mWorldsShouldExist = true;
    }
    
    void PhysicsThread::removePhysicsManager(){
        //The world is destroyed
        std::unique_lock<std::mutex>checkLock();
        
        //Clear the physics pointer. This might cause destruction.
        mPhysicsManager.reset();
        
        mPhysicsManagerReady = false;
        mWorldsShouldExist = false;
    }
}
