#include "PhysicsManager.h"

#include "Worlds/DynamicsWorld.h"
#include "Worlds/CollisionWorld.h"

#include "PhysicsShapeManager.h"
#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"

#include "Worlds/CollisionWorldData/CollisionWorldDataManager.h"
#include "System/SystemSetup/SystemSettings.h"
#include "PhysicsCollisionDataManager.h"

namespace AV{
    PhysicsManager::PhysicsManager(){
        for(int i = 0; i < MAX_COLLISION_WORLDS; i++){
            mCollisionWorlds[i].reset();
        }
        initialise();
    }

    PhysicsManager::~PhysicsManager(){
        EventDispatcher::unsubscribe(EventType::World, this);

        PhysicsCollisionDataManager::shutdown();
    }

    void PhysicsManager::update(){
        mDynamicsWorld->update();

        for(int i = 0; i < mCreatedCollisionWorlds; i++){
            mCollisionWorlds[i]->update();
        }
    }

    void PhysicsManager::initialise(){
        mDynamicsWorld = std::make_shared<DynamicsWorld>();

        mCollisionDataManager = std::make_shared<CollisionWorldDataManager>();

        mCreatedCollisionWorlds = SystemSettings::getNumCollisionWorlds();
        for(int i = 0; i < mCreatedCollisionWorlds; i++){
            mCollisionWorlds[i] = std::make_shared<CollisionWorld>(i, mCollisionDataManager);
        }

        EventDispatcher::subscribe(EventType::World, AV_BIND(PhysicsManager::worldEventReceiver));
    }

    CollisionWorld* PhysicsManager::getCollisionWorld(uint8 worldId){
        if(worldId > mCreatedCollisionWorlds) return 0;

        return mCollisionWorlds[worldId].get();
    }

    bool PhysicsManager::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        if(event.eventCategory() == WorldEventCategory::OriginChange){
            const WorldEventOriginChange& originEvent = (WorldEventOriginChange&)event;

            mDynamicsWorld->notifyOriginShift(originEvent.worldOffset);
            for(int i = 0; i < mCreatedCollisionWorlds; i++){
                mCollisionWorlds[i]->notifyOriginShift(originEvent.worldOffset);
            }
        }
        return false;
    }
}
