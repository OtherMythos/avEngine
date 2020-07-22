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

    private:

        static SQObject SceneNodeDelegateTableObject;

        static SQInteger setPosition(HSQUIRRELVM vm);
        static SQInteger setScale(HSQUIRRELVM vm);
        static SQInteger createChildSceneNode(HSQUIRRELVM vm);

        static SQInteger attachObject(HSQUIRRELVM vm);
    };
}
