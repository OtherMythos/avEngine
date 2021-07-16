#pragma once

#include <string>
#include "System/EnginePrerequisites.h"

#include <memory>

class dtNavMesh;

namespace AV{
    typedef uint64 NavMeshId;
    typedef uint64 NavQueryId;

    static const NavMeshId INVALID_NAV_MESH = 0xFFFFFFFFFFFFFFFF;
    static const NavQueryId INVALID_NAV_QUERY = INVALID_NAV_MESH;

    /**
    Reference counted id for tiles.
    When a recipe has finished loading, it will exchange its raw tile data pointers for
    this type of id from the NavMeshManager.
    When all references reach 0, the tile data is destroyed.
    */
    typedef std::shared_ptr<void> NavTileId;

    struct NavMeshTileData{
        //Only populated until the data has been delivered to the chunk manager.
        unsigned char* tileData;
        int dataSize;
        //Populated after the tileData has been yielded to the NavMeshManager.
        NavTileId tileId;
        //Technically these are stored in the tileData but it's useful to have them here.
        int x;
        int y;
        int navMeshId;
    };
}
