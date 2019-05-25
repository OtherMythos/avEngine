#include "ThreadManager.h"

#include "Thread/PhysicsThread.h"
#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"
#include "World/WorldSingleton.h"

#include "Logger/Log.h"

namespace AV{
    ThreadManager::ThreadManager(){
        initialise();
        
        EventDispatcher::subscribe(EventType::World, AV_BIND(ThreadManager::worldEventReceiver));
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
    
    bool ThreadManager::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        //TODO this needs checks as to whether the world is ready or not.
        if(event.eventCategory() == WorldEventCategory::Created){
            const WorldEventCreated& wEvent = (WorldEventCreated&)event;
            mPhysicsThreadInstance->providePhysicsManager(WorldSingleton::getWorldNoCheck()->getPhysicsManager());
            if(!wEvent.createdFromSave){
                //The world wasn't created from a save, which means it's immediately ready.
                mPhysicsThreadInstance->setReady(true);
            }
        }
        if(event.eventCategory() == WorldEventCategory::Destroyed){
            mPhysicsThreadInstance->removePhysicsManager();
            mPhysicsThreadInstance->setReady(false);
        }
        
        if(event.eventCategory() == WorldEventCategory::BecameReady){
            mPhysicsThreadInstance->setReady(true);
        }
        if(event.eventCategory() == WorldEventCategory::BecameUnready){
            mPhysicsThreadInstance->setReady(false);
        }
        
        return true;
    }
}
