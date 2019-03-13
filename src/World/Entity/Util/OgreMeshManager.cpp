#include "OgreMeshManager.h"

#include "Ogre.h"

namespace AV{
    OgreMeshManager::OgreMeshManager(){
        mSceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("Scene Manager");

        mParentEntityNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_DYNAMIC);
    }

    OgreMeshManager::~OgreMeshManager(){

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
        delete object;

        sceneNode->removeAndDestroyAllChildren();
        delete sceneNode;
    }
}
