#pragma once

#include "OgreVector3.h"

namespace AV{
    struct PositionComponent{
        PositionComponent(Ogre::Vector3 pos) : pos(pos) { }

        Ogre::Vector3 pos;
    };
}
