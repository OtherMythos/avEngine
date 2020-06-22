#pragma once

namespace AV{
    #define ENGINE_VERSION_MAJOR 0
    #define ENGINE_VERSION_MINOR 1
    #define ENGINE_VERSION_PATCH 0
    #define ENGINE_VERSION_SUFFIX "unstable"

    //Engine settings
    #define MAX_COLLISION_WORLDS 4

    typedef unsigned int uint8;
    typedef unsigned short uint16;
    typedef unsigned int uint32;

    #define OGRE_TO_BULLET(w) btVector3(w.x, w.y, w.z)
}
