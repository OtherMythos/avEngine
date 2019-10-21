#include "MovableTextureManager.h"

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"

#include "Gui/Rect2d/Rect2dMovable.h"
#include "MovableTexture.h"

namespace AV{
    MovableTextureManager::MovableTextureManager(){

    }

    MovableTextureManager::~MovableTextureManager(){

    }

    void MovableTextureManager::initialise(Ogre::SceneManager* sceneManager){
        mSceneManager = sceneManager;

        mParentNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_DYNAMIC); //TODO destroy this on shutdown.
        mParentNode->setPosition(Ogre::Vector3::ZERO);
    }

    MovableTexture* MovableTextureManager::createTexture(const Ogre::String& resourceName){
        Rect2dMovable* rectMov = static_cast<Rect2dMovable*>(
            mSceneManager->createMovableObject("Rect2dMovable", &mSceneManager->_getEntityMemoryManager(Ogre::SCENE_DYNAMIC))
        );

        Ogre::SceneNode* node = mParentNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        node->attachObject(rectMov);


        //Soon these will be wrapped around a shared pointer to manage deletion.
        MovableTexture* movTex = new MovableTexture(node, rectMov);

        return movTex;
    }
}
