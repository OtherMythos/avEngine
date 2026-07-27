#include "ThreadManager.h"
#include "System/BaseSingleton.h"

#include "Thread/Physics/PhysicsThread.h"
#include "Physics/PhysicsBodyDestructor.h"

#include "Logger/Log.h"

namespace AV{
    ThreadManager::ThreadManager(){
        initialise();
    }

    ThreadManager::~ThreadManager(){
        mPhysicsThreadInstance->shutdown();

        mPhysicsThread->join();
        AV_INFO("Joined physics thread.");

        delete mPhysicsThreadInstance;
        delete mPhysicsThread;
    }

    void ThreadManager::initialise(){
        mPhysicsThreadInstance = new PhysicsThread();

        mPhysicsThread = new std::thread(&PhysicsThread::run, mPhysicsThreadInstance);
    }

    void ThreadManager::notifyPhysicsManagerCreated(std::shared_ptr<PhysicsManager> physicsManager){
        mPhysicsThreadInstance->notifyPhysicsManagerCreated(physicsManager);
        mPhysicsThreadInstance->setReady(true);
    }

    void ThreadManager::notifyPhysicsManagerDestroyed(){
        PhysicsBodyDestructor::notifyPhysicsWorldDestruction();
        mPhysicsThreadInstance->notifyPhysicsManagerDestroyed();
        mPhysicsThreadInstance->setReady(false);
    }

    void ThreadManager::sheduleUpdate(int time){
        mPhysicsThreadInstance->scheduleWorldUpdate(time);
    }
}
