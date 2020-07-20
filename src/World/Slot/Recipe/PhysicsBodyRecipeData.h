#pragma once

/**
In certain cases, code bases will need to borrow things such as parsers.
Southsea, the level editor, uses some of the engine file parsers, but does not have bullet as a dependency.
In that case I work around that by substituting bullet stuff with ogre stuff.
*/
#ifndef NO_BULLET
    #include <LinearMath/btVector3.h>
    #include <LinearMath/btQuaternion.h>
#else
    #include "OgreVector3.h"
    #include "OgreQuaternion.h"
#endif
#include "System/EnginePrerequisites.h"

namespace AV{
    #ifdef NO_BULLET
        typedef Ogre::Vector3 btVector3;
        typedef Ogre::Quaternion btQuaternion;
    #endif

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
