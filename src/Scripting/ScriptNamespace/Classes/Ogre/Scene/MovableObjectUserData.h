#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "OgreObjectTypes.h"

namespace Ogre{
    class SceneNode;
}

namespace AV{
    class MovableObjectUserData{
    public:
        MovableObjectUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void movableObjectToUserData(HSQUIRRELVM vm, Ogre::MovableObject* object, MovableObjectType type);
        static UserDataGetResult readMovableObjectFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::MovableObject** outObject, MovableObjectType expectedType = MovableObjectType::Any);

    private:

        static SQObject itemDelegateTableObject;
        static SQObject lightDelegateTableObject;
        static SQObject particleSystemDelegateTableObject;
        static SQObject cameraDelegateTableObject;

        //Shared between movable objects
        static SQInteger getLocalRadius(HSQUIRRELVM vm);
        static SQInteger getLocalAabb(HSQUIRRELVM vm);
        static SQInteger setVisibilityFlags(HSQUIRRELVM vm);
        static SQInteger setRenderQueueGroup(HSQUIRRELVM vm);

        //Item specific
        static SQInteger setDatablock(HSQUIRRELVM vm);
        static SQInteger itemHasSkeleton(HSQUIRRELVM vm);
        static SQInteger itemGetSkeleton(HSQUIRRELVM vm);

        //Light specific
        static SQInteger setLightType(HSQUIRRELVM vm);
        static SQInteger setLightPowerScale(HSQUIRRELVM vm);
        static SQInteger setLightDiffuseColour(HSQUIRRELVM vm);

        //Camera
        static SQInteger cameraLookAt(HSQUIRRELVM vm);

    };
}
