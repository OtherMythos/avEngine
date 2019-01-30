#pragma once

#include "OgreString.h"
#include "OgreVector3.h"

namespace AV{
    //Recipe data for static ogre mesh recipies.
    struct OgreMeshRecipeData{
        Ogre::String meshName;
        Ogre::Vector3 pos;
        Ogre::Vector3 scale;
    };
};
