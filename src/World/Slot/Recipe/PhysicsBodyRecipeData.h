#pragma once

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include "System/EnginePrerequisites.h"

namespace AV{
    //Recipe data for static ogre mesh recipies.
    struct PhysicsBodyRecipeData{
        int shapeId = 0;
        btVector3 pos;
        btQuaternion orientation;
    };

    struct PhysicsShapeRecipeData{
        int physicsShapeType;
        btVector3 scale;
    };

    //Both script, closure, and packed int info.
    struct CollisionObjectScriptData{
        //I'm limiting the size of the vector to 16 bits.
        //This is to help shrink the size here, and plus it doesn't make much sense to go over this amount.
        uint16 scriptIdx;
        uint16 closureIdx;
    };

    struct CollisionObjectRecipeData{
        uint16 shapeId;
        uint16 scriptId;
        uint16 dataId;

        btVector3 pos;
        btQuaternion orientation;
    };
};
