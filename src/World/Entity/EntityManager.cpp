#include "EntityManager.h"

#include "World/Slot/ChunkRadiusLoader.h"
#include "World/WorldSingleton.h"

#include "Components/PositionComponent.h"
#include "Components/OgreMeshComponent.h"
#include "Components/ScriptComponent.h"

#include "Logic/ComponentLogic.h"
#include "Logic/OgreMeshComponentLogic.h"
#include "Logic/ScriptComponentLogic.h"

#include "Tracker/EntityTracker.h"

#include "Util/OgreMeshManager.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"

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

    void EntityManager::initialise(){
        //I can safely assume here that if an entity manager is being constructed that means a new world is being constructed.
        //So here is the best place to inject the pointer value into the component logic.
        ComponentLogic::entityManager = this;
        ComponentLogic::entityXManager = &ex;

        mOgreMeshManager = std::make_shared<OgreMeshManager>();
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
        bool viableChunk = WorldSingleton::getWorld()->getChunkRadiusLoader()->chunkLoadedInCurrentMap(pos.chunkX(), pos.chunkY());
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

    void EntityManager::setEntityPosition(eId id, SlotPosition position){
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
    
    void EntityManager::_repositionEntityOriginSwitch(){
        ex.entities.each<OgreMeshComponent>([](entityx::Entity entity, OgreMeshComponent &comp) {
            OgreMeshComponentLogic::reposition(_eId(entity));
        });
    }
    
    void EntityManager::_mapChange(){
        mEntityTracker->destroyTrackedEntities();
    }
    
    bool EntityManager::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        if(event.eventCategory() == WorldEventCategory::OriginChange){
            _repositionEntityOriginSwitch();
        }
        if(event.eventCategory() == WorldEventCategory::MapChange){
            _mapChange();
        }
        return true;
    }
}
