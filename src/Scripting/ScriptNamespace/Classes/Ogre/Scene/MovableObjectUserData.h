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

        //Shared between movable objects, so also registered by the per-type delegate tables (e.g. CameraUserData).
        static SQInteger getParentNode(HSQUIRRELVM vm);

    private:

        static SQObject itemDelegateTableObject;
        static SQObject lightDelegateTableObject;
        static SQObject particleSystemDelegateTableObject;

        //Shared between movable objects
        static SQInteger getLocalRadius(HSQUIRRELVM vm);
        static SQInteger getLocalAabb(HSQUIRRELVM vm);
        static SQInteger getWorldAabb(HSQUIRRELVM vm);
        static SQInteger getWorldAabbUpdated(HSQUIRRELVM vm);
        static SQInteger getWorldRadius(HSQUIRRELVM vm);
        static SQInteger setVisibilityFlags(HSQUIRRELVM vm);
        static SQInteger setRenderQueueGroup(HSQUIRRELVM vm);
        static SQInteger setQueryFlags(HSQUIRRELVM vm);

        //Item specific
        static SQInteger setDatablock(HSQUIRRELVM vm);
        static SQInteger itemHasSkeleton(HSQUIRRELVM vm);
        static SQInteger itemGetSkeleton(HSQUIRRELVM vm);
        static SQInteger itemUseSkeletonInstanceFrom(HSQUIRRELVM vm);
        static SQInteger setCastsShadows(HSQUIRRELVM vm);
        static SQInteger getItemName(HSQUIRRELVM vm);

        //Light specific
        static SQInteger setLightType(HSQUIRRELVM vm);
        static SQInteger setLightPowerScale(HSQUIRRELVM vm);
        static SQInteger setLightDiffuseColour(HSQUIRRELVM vm);
        static SQInteger setLightSpecularColour(HSQUIRRELVM vm);
        static SQInteger setLightAttenuationBasedOnRadius(HSQUIRRELVM vm);
        static SQInteger setLightDirection(HSQUIRRELVM vm);
        static SQInteger setShadowFarDistance(HSQUIRRELVM vm);
        static SQInteger setShadowFarClipDistance(HSQUIRRELVM vm);
        static SQInteger setShadowNearClipDistance(HSQUIRRELVM vm);

        //Particle System
        static SQInteger particleSystemFastForward(HSQUIRRELVM vm);
        static SQInteger particleSystemSetEmitting(HSQUIRRELVM vm);
        static SQInteger particleSystemGetEmitting(HSQUIRRELVM vm);
    };
}
