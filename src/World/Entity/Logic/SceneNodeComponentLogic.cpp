#include "SceneNodeComponentLogic.h"

#include "OgreSceneNode.h"
#include "World/Entity/Logic/SceneNodeComponentLogic.h"
#include "World/Entity/Components/SceneNodeComponent.h"

#include "entityx/entityx.h"

namespace AV{
    void SceneNodeComponentLogic::add(eId id, Ogre::SceneNode* targetNode, const Ogre::Vector3& targetPos){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(entity.has_component<SceneNodeComponent>()) return;

        targetNode->setPosition(targetPos);
        entity.assign<SceneNodeComponent>(targetNode);
    }

    bool SceneNodeComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<SceneNodeComponent> compMesh = entity.component<SceneNodeComponent>();
        if(compMesh){
            entity.remove<SceneNodeComponent>();
            return true;
        }

        return false;
    }

    Ogre::SceneNode* SceneNodeComponentLogic::getSceneNode(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<SceneNodeComponent> nodeComp = entity.component<SceneNodeComponent>();
        if(!nodeComp){
            return 0;
        }
        return nodeComp.get()->node;
    }

    void SceneNodeComponentLogic::repositionKnown(eId id, const SlotPosition& pos){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        /**
        OPTIMISATION The toOgre could be called further up the stack.
        */
        entityx::ComponentHandle<SceneNodeComponent> nodeComp = entity.component<SceneNodeComponent>();
        if(nodeComp) nodeComp.get()->node->setPosition(pos.toOgre());
    }
}
