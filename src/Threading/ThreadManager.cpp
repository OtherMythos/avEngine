#include "ThreadManager.h"
#include <cmath>

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


    bool ThreadManager::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;

        switch(event.eventId()){
            case EventId::WorldCreated:{
                const WorldEventCreated& wEvent = (WorldEventCreated&)event;

                mPhysicsThreadInstance->notifyWorldCreation(WorldSingleton::getWorldNoCheck()->getPhysicsManager());

                if(!wEvent.createdFromSave){
                    //The world wasn't created from a save, which means it's immediately ready.
                    mPhysicsThreadInstance->setReady(true);
                }else{
                    mPhysicsThreadInstance->setReady(false);
                }
                break;
            }
            case EventId::WorldDestroyed:{
                mPhysicsThreadInstance->notifyWorldDestruction();
                mPhysicsThreadInstance->setReady(false);
                break;
            }

            case EventId::WorldBecameReady:{
                mPhysicsThreadInstance->setReady(true);
                break;
            }
            case EventId::WorldBecameUnready:{
                mPhysicsThreadInstance->setReady(false);
                break;
            }
            default:
                break;
        }

        return true;
    }

    void ThreadManager::schedulePhysicsStep(double deltaSeconds){
        //Rounded, not truncated. See the matching comment in PhysicsThread::_initTimestepIfNeeded.
        mPhysicsThreadInstance->scheduleStep(llround(deltaSeconds * 1000000000.0));
    }

    void ThreadManager::waitForPhysicsStep(){
        mPhysicsThreadInstance->waitForScheduledStep();
    }
}
