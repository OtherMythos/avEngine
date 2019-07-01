#pragma once

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

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
};
