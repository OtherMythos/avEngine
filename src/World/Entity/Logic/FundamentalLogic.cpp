#include "FundamentalLogic.h"

#include "entityx/entityx.h"

#include "World/Entity/EntityManager.h"
#include "World/Entity/Components/PositionComponent.h"

namespace AV{
    SlotPosition FundamentalLogic::getPosition(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<PositionComponent> compPos = entity.component<PositionComponent>();
        return compPos.get()->pos;
    }

    bool FundamentalLogic::getTracked(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        entityx::ComponentHandle<PositionComponent> compPos = entity.component<PositionComponent>();
        
        return compPos.get()->tracked;
    }
    
    void FundamentalLogic::setTracked(eId id, bool tracked){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        entityx::ComponentHandle<PositionComponent> compPos = entity.component<PositionComponent>();
        
        compPos.get()->tracked = tracked;
    }
}
