#include "OgreMeshManager.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"
#include "World/Support/Obj/ObjMeshLoader.h"
#include "Ogre.h"

#include "OgreItem.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreResourceGroupManager.h"

namespace AV{
    Ogre::SceneManager* OgreMeshManager::mSceneManager;
    std::vector<OgreMeshManager::MeshFormat> OgreMeshManager::mMeshFormats;

    OgreMeshManager::OgreMeshManager(){
        EventDispatcher::subscribe(EventType::World, AV_BIND(OgreMeshManager::worldEventReceiver));
    }

    OgreMeshManager::~OgreMeshManager(){
        //The code to destroy meshes should now be called when the pointers are destroyed.
        //_iterateAndDestroy(mParentEntityNode);
        //mParentEntityNode->removeAndDestroyAllChildren();
        //mSceneManager->destroySceneNode(mParentEntityNode);
    }

    void OgreMeshManager::setupSceneManager(Ogre::SceneManager* manager){
        mSceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("Scene Manager");

        mParentEntityNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_DYNAMIC);

        //Register the mesh file formats the engine ships with.
        registerMeshFormat(".obj", &ObjMeshLoader::getLoader());
    }

    void OgreMeshManager::registerMeshFormat(const Ogre::String& extension, Ogre::ManualResourceLoader* loader){
        mMeshFormats.push_back({extension, loader});
    }

    void OgreMeshManager::ensureMeshRegistered(const Ogre::String& meshName, const Ogre::String& groupName){
        Ogre::ManualResourceLoader* loader = 0;
        for(const MeshFormat& format : mMeshFormats){
            if(Ogre::StringUtil::endsWith(meshName, format.extension, true)){
                loader = format.loader;
                break;
            }
        }
        //A regular .mesh file, or an unknown extension. Leave it to Ogre's default handling.
        if(!loader) return;

        Ogre::MeshManager& meshManager = Ogre::MeshManager::getSingleton();
        if(meshManager.getByName(meshName, groupName)) return;

        Ogre::String targetGroup = groupName;
        if(targetGroup == Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME){
            targetGroup = Ogre::ResourceGroupManager::getSingleton().findGroupContainingResource(meshName);
        }

        meshManager.createManual(meshName, targetGroup, loader);
    }

    Ogre::Item* OgreMeshManager::createItem(Ogre::SceneManager* sceneManager, const Ogre::String& meshName,
        const Ogre::String& groupName, Ogre::SceneMemoryMgrTypes sceneType){
        ensureMeshRegistered(meshName, groupName);
        return sceneManager->createItem(meshName, groupName, sceneType);
    }

    void OgreMeshManager::_destroyOgreMesh(Ogre::SceneNode* sceneNode){
        if(!sceneNode){
            return;
        }
        Ogre::MovableObject* object = sceneNode->getAttachedObject(0);
        mSceneManager->destroyMovableObject(object);

        sceneNode->removeAndDestroyAllChildren();
        mSceneManager->destroySceneNode(sceneNode);
    }

    void OgreMeshManager::_iterateAndDestroy(Ogre::SceneNode* node){
        //At the moment the nodes only contain a single level of other nodes.
        //This means here I only need to traverse the top layer.
        //If in future there are other nodes in lower layers they'll need to be deleted as well, and this would become a recursive destroyer.

        auto it = node->getChildIterator();
        while(it.current() != it.end()){
            Ogre::SceneNode *node = (Ogre::SceneNode*)it.getNext();

            _destroyOgreMesh(node);
        }
    }

    void OgreMeshManager::gatherMeshSerialisationData(std::vector<SerialisedMeshEntry>& serialisedMeshes){
        SerialisedMeshEntry entry;

        auto it = mParentEntityNode->getChildIterator();
        while(it.current() != it.end()){
            Ogre::SceneNode *node = (Ogre::SceneNode*)it.getNext();
            Ogre::Item* item = (Ogre::Item*) node->getAttachedObject(0);

            entry.nodePtr = node;
            entry.meshName = item->getMesh()->getName();
            entry.pos = node->getPosition();
            entry.orientation = node->getOrientation();

            serialisedMeshes.push_back(entry);
        }
    }

    OgreMeshManager::OgreMeshPtr OgreMeshManager::createMesh(const Ogre::String& meshName){
        Ogre::SceneNode *node = mParentEntityNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        Ogre::Item *item = createItem(mSceneManager, meshName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
        node->attachObject((Ogre::MovableObject*)item);

        OgreMeshPtr meshPtr(node, _destroyOgreMesh);

        return meshPtr;
    }

    void OgreMeshManager::_repositionMeshesOriginShift(Ogre::Vector3 offset){
        auto it = mParentEntityNode->getChildIterator();
        while(it.current() != it.end()){
            Ogre::SceneNode *node = (Ogre::SceneNode*)it.getNext();

            node->setPosition(node->getPosition() - offset);
        }
    }

    bool OgreMeshManager::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        if(event.eventId() == EventId::WorldOriginChange){
            const WorldEventOriginChange& originEvent = (WorldEventOriginChange&)event;

            _repositionMeshesOriginShift(originEvent.worldOffset);
        }
        return true;
    }
}
