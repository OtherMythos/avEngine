#include "EntityManager.h"

#include "World/Slot/ChunkRadiusLoader.h"
#include "World/WorldSingleton.h"

#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

#include "Components/PositionComponent.h"
#include "Components/OgreMeshComponent.h"
#include "Components/ScriptComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/CollisionComponent.h"

#include "Logic/ComponentLogic.h"
#include "Logic/OgreMeshComponentLogic.h"
#include "Logic/ScriptComponentLogic.h"
#include "Logic/CollisionComponentLogic.h"

#include "Tracker/EntityTracker.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"
#include "System/SystemSetup/SystemSettings.h"
#include "World/Entity/Components/NavigationComponent.h"
#include "World/Entity/Logic/NavigationComponentLogic.h"

#include "Callback/EntityCallbackManager.h"

#include "Logger/Log.h"

namespace AV{
    EntityManager::EntityManager(){

    }

    EntityManager::~EntityManager(){
        AV_INFO("Shutting down the Entity Manager.");

        EventDispatcher::unsubscribe(EventType::World, this);

        ComponentLogic::entityManager = 0;
        ComponentLogic::entityXManager = 0;
    }

    void EntityManager::update(){
        if(!SystemSettings::getDynamicPhysicsDisabled()){
            const std::vector<DynamicsWorld::EntityTransformData>& data = mPhysicsManager->getDynamicsWorld()->getEntityTransformData();
            for(const DynamicsWorld::EntityTransformData& e : data){
                Ogre::Vector3 pos(e.pos.x(), e.pos.y(), e.pos.z());
                //The true flag will make sure that the rigid body is not updated by this move.
                setEntityPosition(e.entity, SlotPosition(pos), true);
            }
        }

        //Loop over all entities with a navigation component
        ex.entities.each<NavigationComponent>([](entityx::Entity entity, NavigationComponent &nav){
            NavigationComponentLogic::updatePathFinding(_eId(entity));
        });
    }

    void EntityManager::initialise(){
        //I can safely assume here that if an entity manager is being constructed that means a new world is being constructed.
        //So here is the best place to inject the pointer value into the component logic.
        ComponentLogic::entityManager = this;
        ComponentLogic::entityXManager = &ex;

        mEntityTracker = std::make_shared<EntityTracker>();
        mEntityCallbackManager = std::make_shared<EntityCallbackManager>();
        mEntityTracker->initialise(this);

        EventDispatcher::subscribe(EventType::World, AV_BIND(EntityManager::worldEventReceiver));
    }

    entityx::Entity EntityManager::_createEntity(SlotPosition pos, bool tracked){
        AV_INFO("Creating entity at position {}", pos);
        entityx::Entity entity = ex.entities.create();

        entity.assign<PositionComponent>(pos, tracked);

        return entity;
    }

    eId EntityManager::createEntity(SlotPosition pos){
        return _eId(_createEntity(pos, false));
    }

    eId EntityManager::createEntityTracked(SlotPosition pos){
        //If the entity is not going to be created in a viable chunk then it can't be tracked. Don't even create it.
        bool viableChunk = WorldSingleton::getWorldNoCheck()->getChunkRadiusLoader()->chunkLoadedInCurrentMap(pos.chunkX(), pos.chunkY());
        if(!viableChunk) return eId::INVALID;

        eId entity = _eId(_createEntity(pos, true));
        mEntityTracker->trackKnownEntity(entity, pos);

        return entity;
    }

    void EntityManager::destroyKnownEntity(eId entity, bool tracked){
        AV_INFO("Destroying entity {}", entity.id());

        //Send the event first, so that all the entity state is before the destruction.
        notifyEntityEvent(entity, EntityEventType::DESTROYED);

        if(tracked){
            mEntityTracker->untrackEntity(entity);
        }

        entityx::Entity e = getEntityHandle(entity);
        entityx::ComponentHandle<OgreMeshComponent> meshComponent = e.component<OgreMeshComponent>();
        if(meshComponent) OgreMeshComponentLogic::remove(entity);

        entityx::ComponentHandle<ScriptComponent> scriptComponent = e.component<ScriptComponent>();
        if(scriptComponent) ScriptComponentLogic::remove(entity);

        e.destroy();
    }

    void EntityManager::destroyEntity(eId entity){
        if(entity == eId::INVALID) return;
        entityx::Entity e = getEntityHandle(entity);
        entityx::ComponentHandle<PositionComponent> compPos = e.component<PositionComponent>();
        bool tracked = false;
        if(compPos){
            if(compPos.get()->tracked) tracked = true;
        }

        destroyKnownEntity(entity, tracked);
    }

    void EntityManager::setEntityOrientation(eId id, Ogre::Quaternion orientation){
        OgreMeshComponentLogic::orientate(id, orientation);
    }

    void EntityManager::setEntityPosition(eId id, SlotPosition position, bool autoMove){
        entityx::Entity e = getEntityHandle(id);
        if(!e.valid()) return;

        entityx::ComponentHandle<PositionComponent> compPos = e.component<PositionComponent>();

        if(compPos.get()->tracked){
            if(!mEntityTracker->updateEntity(id, compPos.get()->pos, position)) return;
        }
        if(compPos){
            compPos.get()->pos = position;
        }

        Ogre::Vector3 absPos = position.toOgre();
        if(e.has_component<OgreMeshComponent>()){
            OgreMeshComponentLogic::repositionKnown(id, absPos);
        }
        if(!autoMove){
            entityx::ComponentHandle<RigidBodyComponent> rigidBody = e.component<RigidBodyComponent>();
            if(rigidBody){
                //If the entity has a rigid body component then physics should exist.
                assert(!SystemSettings::getDynamicPhysicsDisabled());
                btVector3 btAbsPos(absPos.x, absPos.y, absPos.z);
                mPhysicsManager->getDynamicsWorld()->setBodyPosition(rigidBody.get()->body, btAbsPos);
            }
        }
        if(e.has_component<CollisionComponent>()){
            CollisionComponentLogic::repositionKnown(id, absPos);
        }

        notifyEntityEvent(id, EntityEventType::MOVED);
    }

    void EntityManager::notifyEntityEvent(eId entity, EntityEventType event){
        entityx::Entity e = getEntityHandle(entity);
        if(!e.valid()) return;

        entityx::ComponentHandle<ScriptComponent> comp = e.component<ScriptComponent>();
        if(comp){
            mEntityCallbackManager->notifyEvent(entity, event, comp.get()->scriptId);
        }
    }

    bool EntityManager::getEntityValid(eId entity){
        entityx::Entity e = getEntityHandle(entity);
        if(e.valid()) return true;

        return false;
    }

    void EntityManager::getDebugInfo(EntityDebugInfo *info){
        info->totalEntities = ex.entities.size();
        info->trackedEntities = mEntityTracker->getTrackedEntities();
        info->trackingChunks = mEntityTracker->getTrackingChunks();
        info->totalCallbackScripts = mEntityCallbackManager->getActiveScripts();
    }

    void EntityManager::_mapChange(){
        mEntityTracker->destroyTrackedEntities();
    }

    bool EntityManager::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        if(event.eventId() == EventId::WorldMapChange){
            _mapChange();
        }
        return true;
    }
}
