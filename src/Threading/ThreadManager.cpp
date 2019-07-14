#include "ThreadManager.h"

#include "Thread/Physics/PhysicsThread.h"
#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"
#include "World/WorldSingleton.h"

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

        EventDispatcher::subscribe(EventType::World, AV_BIND(ThreadManager::worldEventReceiver));
    }

    void ThreadManager::sheduleUpdate(int time){
        mPhysicsThreadInstance->scheduleWorldUpdate(time);
    }

    bool ThreadManager::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;

        if(event.eventCategory() == WorldEventCategory::Created){
            const WorldEventCreated& wEvent = (WorldEventCreated&)event;

            mPhysicsThreadInstance->notifyWorldCreation(WorldSingleton::getWorldNoCheck()->getPhysicsManager());

            if(!wEvent.createdFromSave){
                //The world wasn't created from a save, which means it's immediately ready.
                mPhysicsThreadInstance->setReady(true);
            }else{
                mPhysicsThreadInstance->setReady(false);
            }
        }
        if(event.eventCategory() == WorldEventCategory::Destroyed){
            mPhysicsThreadInstance->notifyWorldDestruction();
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
