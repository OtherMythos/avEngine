#pragma once

#include "OgreVector3.h"

namespace AV{
    typedef unsigned char DataPointType;
    typedef unsigned char DataPointSubType;

    struct DataPointEntry{
        Ogre::Vector3 pos;
        DataPointType type;
        DataPointSubType subType;
        int userData;
    };
}
