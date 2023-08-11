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
        static SQInteger setQueryFlags(HSQUIRRELVM vm);
        static SQInteger getParentNode(HSQUIRRELVM vm);

        //Item specific
        static SQInteger setDatablock(HSQUIRRELVM vm);
        static SQInteger itemHasSkeleton(HSQUIRRELVM vm);
        static SQInteger itemGetSkeleton(HSQUIRRELVM vm);
        static SQInteger itemUseSkeletonInstanceFrom(HSQUIRRELVM vm);
        static SQInteger setCastsShadows(HSQUIRRELVM vm);

        //Light specific
        static SQInteger setLightType(HSQUIRRELVM vm);
        static SQInteger setLightPowerScale(HSQUIRRELVM vm);
        static SQInteger setLightDiffuseColour(HSQUIRRELVM vm);
        static SQInteger setLightSpecularColour(HSQUIRRELVM vm);
        static SQInteger setLightAttenuationBasedOnRadius(HSQUIRRELVM vm);
        static SQInteger setLightDirection(HSQUIRRELVM vm);

        //Camera
        static SQInteger cameraLookAt(HSQUIRRELVM vm);
        static SQInteger cameraSetProjectionType(HSQUIRRELVM vm);
        static SQInteger cameraSetOrthoWindow(HSQUIRRELVM vm);
        static SQInteger cameraGetWorldPosInWindow(HSQUIRRELVM vm);
        static SQInteger cameraSetAspectRatio(HSQUIRRELVM vm);
        static SQInteger cameraGetCameraToViewportRay(HSQUIRRELVM vm);
        static SQInteger cameraSetDirection(HSQUIRRELVM vm);
        static SQInteger cameraGetOrientation(HSQUIRRELVM vm);
    };
}
