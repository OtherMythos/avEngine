#include "FundamentalLogic.h"

#include "entityx/entityx.h"

#include "World/Entity/EntityManager.h"
#include "World/Entity/Components/PositionComponent.h"

#include <fstream>

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

    void FundamentalLogic::serialise(std::ofstream& stream, entityx::Entity& e){
        entityx::ComponentHandle<PositionComponent> comp = e.component<PositionComponent>();

        SlotPosition pos = comp.get()->pos;
        stream << "[Position]\n";
        stream << pos.chunkX() << std::endl;
        stream << pos.chunkY() << std::endl;

        Ogre::Vector3 p = pos.position();
        stream << p.x << " " << p.y << " " << p.z << std::endl;

        stream << comp.get()->tracked << std::endl;
    }
}
