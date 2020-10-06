#pragma once

namespace AV{
    #define ENGINE_VERSION_MAJOR 0
    #define ENGINE_VERSION_MINOR 1
    #define ENGINE_VERSION_PATCH 0
    #define ENGINE_VERSION_SUFFIX "unstable"

    //Engine settings
    #define MAX_COLLISION_WORLDS 4

    typedef unsigned char uint8;
    typedef unsigned short uint16;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;

    #define OGRE_TO_BULLET(w) btVector3(w.x, w.y, w.z)
    #define OGRE_TO_BULLET_QUAT(m) btQuaternion(m.w, m.x, m.y, m.z)
}
