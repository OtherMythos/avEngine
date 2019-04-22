#include "OgreMeshManager.h"

#include "Ogre.h"

namespace AV{
    OgreMeshManager::OgreMeshManager(){
        mSceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("Scene Manager");

        mParentEntityNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_DYNAMIC);
    }

    OgreMeshManager::~OgreMeshManager(){
        _iterateAndDestroy(mParentEntityNode);
        
        mParentEntityNode->removeAndDestroyAllChildren();
        
        mSceneManager->destroySceneNode(mParentEntityNode);
    }
    
    void OgreMeshManager::_iterateAndDestroy(Ogre::SceneNode* node){
        //At the moment the nodes only contain a single level of other nodes.
        //This means here I only need to traverse the top layer.
        //If in future there are other nodes in lower layers they'll need to be deleted as well, and this would become a recursive destroyer.
        auto it = node->getChildIterator();
        while(it.hasMoreElements()){
            Ogre::SceneNode *node = (Ogre::SceneNode*)it.getNext();
            
            destroyOgreMesh(node);
        }
    }

    Ogre::SceneNode* OgreMeshManager::createOgreMesh(const Ogre::String& meshName){
        Ogre::SceneNode *node = mParentEntityNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        Ogre::Item *item = mSceneManager->createItem(meshName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
        node->attachObject((Ogre::MovableObject*)item);

        return node;
    }

    void OgreMeshManager::destroyOgreMesh(Ogre::SceneNode* sceneNode){
        if(!sceneNode){
            return;
        }
        Ogre::MovableObject* object = sceneNode->getAttachedObject(0);
        mSceneManager->destroyMovableObject(object);

        sceneNode->removeAndDestroyAllChildren();
        mSceneManager->destroySceneNode(sceneNode);
    }
}
