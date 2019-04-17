#include "EntityManager.h"

#include "Components/PositionComponent.h"
#include "Components/OgreMeshComponent.h"

#include "Logic/ComponentLogic.h"
#include "Logic/OgreMeshComponentLogic.h"

#include "Tracker/EntityTracker.h"

#include "Util/OgreMeshManager.h"
#include "OgreSceneNode.h"

#include "Logger/Log.h"

namespace AV{
    EntityManager::EntityManager(){

    }

    EntityManager::~EntityManager(){
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
    }

    eId EntityManager::createEntity(SlotPosition pos){
        AV_INFO("Creating entity at position {}", pos);
        entityx::Entity entity = ex.entities.create();

        entity.assign<PositionComponent>(pos);

        return _eId(entity, false);
    }

    eId EntityManager::createEntityTracked(SlotPosition pos){
        eId entity = createEntity(pos);
        mEntityTracker->trackEntity(entity, pos);

        return eId(entity.id(), true);
    }

    void EntityManager::destroyEntity(eId entity){
        AV_INFO("Destroying entity");
        entityx::Entity e = getEntityHandle(entity);
        e.destroy();
    }

    void EntityManager::setEntityPosition(eId id, SlotPosition position){
        entityx::Entity e = getEntityHandle(id);
        if(!e.valid()) return;
        
        entityx::ComponentHandle<PositionComponent> compPos = e.component<PositionComponent>();

        if(id.tracked()){
            if(!mEntityTracker->updateEntity(id, compPos.get()->pos, position, this)) return;
        }
        if(compPos){
            compPos.get()->pos = position;
        }

        Ogre::Vector3 absPos = position.toOgre();
        entityx::ComponentHandle<OgreMeshComponent> meshComp = e.component<OgreMeshComponent>();
        if(meshComp){
            meshComp.get()->parentNode->setPosition(absPos);
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
    }
}
