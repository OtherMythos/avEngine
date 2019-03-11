#include "EntityManager.h"

#include "Components/PositionComponent.h"
#include "Logic/ComponentLogic.h"
#include "Logic/OgreMeshComponentLogic.h"

namespace AV{
    EntityManager::EntityManager(){

    }

    EntityManager::~EntityManager(){

    }

    void EntityManager::initialise(){
        //I can safely assume here that if an entity manager is being constructed that means a new world is being constructed.
        //So here is the best place to inject the pointer value into the component logic.
        ComponentLogic::entityManager = this;
        ComponentLogic::entityXManager = &ex;

        //Example and test.
        SlotPosition pos;
        eId e = createEntity(pos);
        OgreMeshComponentLogic::add(e, "Something");
    }

    eId EntityManager::createEntity(SlotPosition pos){
        entityx::Entity entity = ex.entities.create();

        entity.assign<PositionComponent>(pos);

        return _eId(entity);
    }

    void EntityManager::destroyEntity(eId entity){
        entityx::Entity e = getEntityHandle(entity);
        e.destroy();
    }

    void EntityManager::setEntityPosition(eId id, SlotPosition position){
        entityx::Entity e = getEntityHandle(id);
        entityx::ComponentHandle<PositionComponent> compPos = e.component<PositionComponent>();

        if(compPos){
            compPos.get()->pos = position;
        }
    }
}
