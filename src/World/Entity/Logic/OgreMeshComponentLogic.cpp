#include "OgreMeshComponentLogic.h"

#include "World/Entity/Components/OgreMeshComponent.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/Util/OgreMeshManager.h"

#include "World/Entity/Components/PositionComponent.h"
#include "OgreSceneNode.h"

#include "entityx/entityx.h"

namespace AV{
    void OgreMeshComponentLogic::add(eId id, const Ogre::String &mesh){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(entity.has_component<OgreMeshComponent>()) return;

        Ogre::SceneNode* sceneNode = entityManager->getMeshManager()->createOgreMesh(mesh);

        //Set the position of the mesh to the position of the entity.
        entityx::ComponentHandle<PositionComponent> compPos = entity.component<PositionComponent>();
        if(compPos){
            sceneNode->setPosition(compPos.get()->pos.toOgre());
        }

        entity.assign<OgreMeshComponent>(sceneNode);
    }

    bool OgreMeshComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<OgreMeshComponent> compMesh = entity.component<OgreMeshComponent>();
        if(compMesh){

            entityManager->getMeshManager()->destroyOgreMesh(compMesh.get()->parentNode);
            entity.remove<OgreMeshComponent>();
            return true;
        }

        return false;
    }
}
