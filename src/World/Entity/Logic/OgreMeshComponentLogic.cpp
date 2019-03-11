#include "OgreMeshComponentLogic.h"

#include "World/Entity/Components/OgreMeshComponent.h"

#include "entityx/entityx.h"

namespace AV{
    void OgreMeshComponentLogic::add(eId id, const Ogre::String &mesh){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entity.assign<OgreMeshComponent>((Ogre::SceneNode*)0);
    }
}
