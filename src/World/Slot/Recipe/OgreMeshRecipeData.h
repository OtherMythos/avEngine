#pragma once

#include "OgreString.h"
#include "OgreVector3.h"
#include "OgreIdString.h"

namespace AV{
    //Recipe data for static ogre mesh recipies.
    struct OgreMeshRecipeData{
        Ogre::String meshName;
        Ogre::Vector3 pos;
        Ogre::Vector3 scale;
    };

    struct OgreMeshRecipeDataNew{
        //Even though there's one value here this is in a struct incase I want to add things in the future.
        Ogre::IdString meshName;
    };
};
