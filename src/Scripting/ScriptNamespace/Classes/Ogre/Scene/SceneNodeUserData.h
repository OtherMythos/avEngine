#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class SceneNode;
}

namespace AV{
    class SceneNodeUserData{
    public:
        SceneNodeUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void sceneNodeToUserData(HSQUIRRELVM vm, Ogre::SceneNode* node);
        static UserDataGetResult readSceneNodeFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::SceneNode** outNode);

        static void tagPointToUserData(HSQUIRRELVM vm, Ogre::TagPoint* node);
        static UserDataGetResult readTagPointFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TagPoint** outNode);

    private:

        static SQObject SceneNodeDelegateTableObject;
        static SQObject TagPointDelegateTableObject;

        static SQInteger setPosition(HSQUIRRELVM vm);
        static SQInteger setScale(HSQUIRRELVM vm);
        static SQInteger createChildSceneNode(HSQUIRRELVM vm);
        static SQInteger move(HSQUIRRELVM vm);

        static SQInteger getPosition(HSQUIRRELVM vm);
        static SQInteger getPositionAsVec3(HSQUIRRELVM vm);
        static SQInteger getScale(HSQUIRRELVM vm);
        static SQInteger getOrientation(HSQUIRRELVM vm);

        static SQInteger setVisible(HSQUIRRELVM vm);
        static SQInteger setOrientation(HSQUIRRELVM vm);
        static SQInteger translateNode(HSQUIRRELVM vm);

        static SQInteger nodeYaw(HSQUIRRELVM vm);
        static SQInteger nodeRoll(HSQUIRRELVM vm);
        static SQInteger nodePitch(HSQUIRRELVM vm);

        static SQInteger attachObject(HSQUIRRELVM vm);
        static SQInteger detachObject(HSQUIRRELVM vm);
        static SQInteger getParent(HSQUIRRELVM vm);

        static SQInteger destroyNodeAndChildren(HSQUIRRELVM vm);
        static SQInteger recursiveDestroyChildren(HSQUIRRELVM vm);
        static SQInteger recursiveDestroyAttachedObjects(HSQUIRRELVM vm);

        static SQInteger getNumChildren(HSQUIRRELVM vm);
        static SQInteger getNumAttachedObjects(HSQUIRRELVM vm);
        static SQInteger getChildByIndex(HSQUIRRELVM vm);
        static SQInteger getAttachedObjectByIndex(HSQUIRRELVM vm);

        static SQInteger lookAt(HSQUIRRELVM vm);

        //Tag point
        static SQInteger createChildTagPoint(HSQUIRRELVM vm);
    };
}
