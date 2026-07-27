#include "FundamentalLogic.h"

#include "entityx/entityx.h"

#include "Entity/EntityManager.h"
#include "Entity/Components/PositionComponent.h"

#include <fstream>

namespace AV{
    Ogre::Vector3 FundamentalLogic::getPosition(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<PositionComponent> compPos = entity.component<PositionComponent>();
        return compPos.get()->pos;
    }

}
