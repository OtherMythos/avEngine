#ifdef DEBUGGING_TOOLS

#include "DebugDrawer.h"

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "World/Slot/SlotPosition.h"

namespace AV{
    DebugDrawer::DebugDrawer(){

    }

    DebugDrawer::~DebugDrawer(){

    }

    void DebugDrawer::initialise(Ogre::SceneManager* sceneManager){
        mSceneManager = sceneManager;

        mParentNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
    }

    void DebugDrawer::resetDraw(){
        //If nothing changed in the scene, there's no point resetting anything.
        if(!mShapeChanged) return;

        for(Ogre::SceneNode* n : mPointObjects){
            n->setVisible(false);
        }

        mPointObjectsUsed = 0;
        mShapeChanged = false;
    }

    void DebugDrawer::drawPoint(const SlotPosition& first){
        Ogre::SceneNode* obtainedNode = _obtainDrawPoint();
        obtainedNode->setVisible(true);

        Ogre::Vector3 realPos = first.toOgre();
        obtainedNode->setPosition(realPos);

        mShapeChanged = true;
    }

    Ogre::SceneNode* DebugDrawer::_obtainDrawPoint(){
        if(mPointObjectsUsed >= mPointObjects.size()){
            //We need to create a new object, as there are no more available.
            Ogre::SceneNode* newNode = mParentNode->createChildSceneNode();
            Ogre::Item *item = mSceneManager->createItem("cube", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
            newNode->attachObject((Ogre::MovableObject*)item);

            mPointObjects.push_back(newNode);
            mPointObjectsUsed++;

            return newNode;
        }else{
            Ogre::SceneNode* retNode = mPointObjects[mPointObjectsUsed];
            mPointObjectsUsed++;

            return retNode;
        }
    }
}

#endif
