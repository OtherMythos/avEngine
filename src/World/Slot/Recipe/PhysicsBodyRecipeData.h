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
    struct CollisionObjectScriptAndData{
        //Integer produced by collisionWorldUtils.
        int packedInt;
        //I'm limiting the size of the vector to 16 bits.
        //This is to help shrink the size here, and plus it doesn't make much sense to go over this amount.
        uint16 closureIdx;
        uint16 scriptIdx;
    };
};
