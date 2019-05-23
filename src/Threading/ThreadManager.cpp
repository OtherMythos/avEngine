#include "ThreadManager.h"

#include "Thread/PhysicsThread.h"
#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"

#include "Logger/Log.h"

namespace AV{
    ThreadManager::ThreadManager(){
        initialise();
        
        EventDispatcher::subscribe(EventType::World, AV_BIND(ThreadManager::worldEventReceiver));
    }
    
    ThreadManager::~ThreadManager(){
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
        if(event.eventCategory() == WorldEventCategory::Created){
            AV_INFO("Threading World Created.")
        }
        if(event.eventCategory() == WorldEventCategory::Destroyed){
            AV_INFO("Threading World Destroyed.")
        }
        
        return true;
    }
}
