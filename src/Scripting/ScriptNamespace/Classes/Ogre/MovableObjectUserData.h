#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class SceneNode;
}

namespace AV{
    class MovableObjectUserData{
    public:
        MovableObjectUserData() = delete;

        enum class MovableObjectType{
            Any,
            Item,
            Light
        };

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void movableObjectToUserData(HSQUIRRELVM vm, Ogre::MovableObject* object, MovableObjectType type);
        static UserDataGetResult readMovableObjectFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::MovableObject** outObject, MovableObjectType expectedType = MovableObjectType::Any);

    private:

        static SQObject itemDelegateTableObject;
        static SQObject lightDelegateTableObject;

        static SQInteger setDatablock(HSQUIRRELVM vm);

        static SQInteger setLightType(HSQUIRRELVM vm);
        static SQInteger setLightPowerScale(HSQUIRRELVM vm);
        static SQInteger setLightDiffuseColour(HSQUIRRELVM vm);

    };
}
