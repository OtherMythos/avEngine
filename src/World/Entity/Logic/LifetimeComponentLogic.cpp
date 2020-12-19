#include "LifetimeComponentLogic.h"

#include "World/Entity/EntityManager.h"

#include "entityx/entityx.h"

namespace AV{
    void LifetimeComponentLogic::add(eId id, TimeValue lifetime){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        if(entity.has_component<LifetimeComponent>()) return;

        entity.assign<LifetimeComponent>(lifetime);
    }

    bool LifetimeComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        if(!entity.has_component<LifetimeComponent>()) return false;

        entity.remove<LifetimeComponent>();

        return true;
    }
}
