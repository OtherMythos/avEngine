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

        for(int i = 0; i < ObjectType::Max; i++){
            for(Ogre::SceneNode* n : mData[i].objects){
                n->setVisible(false);
            }
            mData[i].objectsUsed = 0;
        }

        mShapeChanged = false;
    }

    void DebugDrawer::drawPoint(const SlotPosition& pos){
        Ogre::SceneNode* obtainedNode = _obtainDrawPoint(ObjectType::Point);
        obtainedNode->setVisible(true);

        Ogre::Vector3 realPos = pos.toOgre();
        obtainedNode->setPosition(realPos);

        mShapeChanged = true;
    }

    void DebugDrawer::drawAxis(const SlotPosition& pos, DrawAxis axis){
        static const Ogre::Quaternion orientationVals[3] = {Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3(0, 0, 1)), Ogre::Quaternion(), Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3(1, 0, 0))};

        Ogre::SceneNode* obtainedNode = _obtainDrawPoint(ObjectType::Axis);
        obtainedNode->setVisible(true);
        obtainedNode->setOrientation(orientationVals[axis]);
        obtainedNode->setScale(Ogre::Vector3(500, 500, 500)); //Make the line long.

        Ogre::Vector3 realPos = pos.toOgre();
        obtainedNode->setPosition(realPos);

        mShapeChanged = true;
    }

    Ogre::SceneNode* DebugDrawer::_obtainDrawPoint(ObjectType type){
        ObjectEntryData& d = mData[type];

        if(d.objectsUsed >= d.objects.size()){
            //We need to create a new object, as there are no more available.
            Ogre::SceneNode* newNode = mParentNode->createChildSceneNode();
            Ogre::Item *item = mSceneManager->createItem(_getTypeMesh(type), Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
            newNode->attachObject((Ogre::MovableObject*)item);

            d.objects.push_back(newNode);
            d.objectsUsed++;

            return newNode;
        }else{
            Ogre::SceneNode* retNode = d.objects[d.objectsUsed];
            d.objectsUsed++;

            return retNode;
        }
    }

    const char* DebugDrawer::_getTypeMesh(ObjectType type) const{
        switch(type){
            case Point: return "linePoint";
            case Axis: return "line";
            default: assert(false);
        }
    }
}

#endif
