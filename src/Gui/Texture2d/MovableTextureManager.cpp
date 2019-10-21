#include "MovableTextureManager.h"

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"

#include "TextureMovable.h"

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
        Ogre::MovableObject* obj = mSceneManager->createMovableObject("TextureMovable", &mSceneManager->_getEntityMemoryManager(Ogre::SCENE_DYNAMIC));

        Ogre::SceneNode* node = mParentNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        node->attachObject(obj);

        TextureMovable* tex = static_cast<TextureMovable*>(obj);
        tex->setDatablock("HlmsUnlit1");
    }
}
