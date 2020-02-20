#pragma once

#include "OgreString.h"
#include "OgreVector3.h"
#include "OgreIdString.h"

namespace AV{
    struct OgreMeshRecipeData{
        //Even though there's one value here this is in a struct incase I want to add things in the future.
        Ogre::IdString meshName;
    };
};
