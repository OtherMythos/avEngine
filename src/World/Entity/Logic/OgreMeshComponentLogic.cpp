#include "OgreMeshComponentLogic.h"

#include "World/Entity/Components/OgreMeshComponent.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/Util/OgreMeshManager.h"

#include "World/Entity/Components/PositionComponent.h"
#include "OgreItem.h"
#include "OgreMesh2.h"
#include "OgreSceneNode.h"

#include "Serialisation/SerialiserStringStore.h"

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

    void OgreMeshComponentLogic::repositionKnown(eId id, const SlotPosition& pos){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<OgreMeshComponent> meshComp = entity.component<OgreMeshComponent>();
        if(meshComp) meshComp.get()->parentNode->setPosition(pos.toOgre());
    }

    void OgreMeshComponentLogic::reposition(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        //Slight duplication between here and the one above, but that means I don't have to create as many entity handles.
        //The original plan was to call the one above from here, but as I have to create an entity handle to get the position anyway I might as well duplicate for efficiency.
        if(entityx::ComponentHandle<OgreMeshComponent> meshComp = entity.component<OgreMeshComponent>()){
            entityx::ComponentHandle<PositionComponent> compPos = entity.component<PositionComponent>();
            meshComp.get()->parentNode->setPosition(compPos.get()->pos.toOgre());
        }
    }

    void OgreMeshComponentLogic::serialise(std::ofstream& stream, entityx::Entity& e){
        entityx::ComponentHandle<OgreMeshComponent> meshComp = e.component<OgreMeshComponent>();

        stream << "[OgreMesh]\n";
        Ogre::Item* item = (Ogre::Item*)meshComp->parentNode->getAttachedObject(0);

        stream << item->getMesh()->getName() << std::endl;
    }
    
    void OgreMeshComponentLogic::deserialise(eId entity, std::ifstream& file, SerialiserStringStore* store){
        std::string line;
        getline(file, line);
        
        store->mStoredStrings.push_back({entity, line});
    }
}
