#include "PhysicsManager.h"

#include "Worlds/DynamicsWorld.h"

#include "PhysicsShapeManager.h"
#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"

namespace AV{
    PhysicsManager::PhysicsManager(){
        initialise();
    }

    PhysicsManager::~PhysicsManager(){
        EventDispatcher::unsubscribe(EventType::World, this);
    }

    void PhysicsManager::update(){
        mDynamicsWorld->update();
    }

    void PhysicsManager::initialise(){
        mDynamicsWorld = std::make_shared<DynamicsWorld>();

        EventDispatcher::subscribe(EventType::World, AV_BIND(PhysicsManager::worldEventReceiver));
    }

    bool PhysicsManager::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        if(event.eventCategory() == WorldEventCategory::OriginChange){
            const WorldEventOriginChange& originEvent = (WorldEventOriginChange&)event;

            mDynamicsWorld->notifyOriginShift(originEvent.worldOffset);
        }
        return false;
    }
}
