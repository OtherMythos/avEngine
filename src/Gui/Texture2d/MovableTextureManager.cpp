#include "MovableTextureManager.h"

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"

#include "Rect2dMovable.h"

namespace AV{
    MovableTextureManager::MovableTextureManager(){

    }

    MovableTextureManager::~MovableTextureManager(){

    }

    void MovableTextureManager::initialise(Ogre::SceneManager* sceneManager){
        mSceneManager = sceneManager;

        mParentNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        mParentNode->setPosition(Ogre::Vector3::ZERO);
    }

    void MovableTextureManager::createTexture(const Ogre::String& resourceName){
        Ogre::MovableObject* obj = mSceneManager->createMovableObject("Rect2dMovable", &mSceneManager->_getEntityMemoryManager(Ogre::SCENE_DYNAMIC));

        Ogre::SceneNode* node = mParentNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        node->attachObject(obj);

        Rect2dMovable* tex = static_cast<Rect2dMovable*>(obj);
        tex->setDatablock("HlmsUnlit1");
    }
}
