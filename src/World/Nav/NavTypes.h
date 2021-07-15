#pragma once

#include <string>
#include "System/EnginePrerequisites.h"

class dtNavMesh;

namespace AV{
    typedef uint64 NavMeshId;
    typedef uint64 NavQueryId;

    static const NavMeshId INVALID_NAV_MESH = 0xFFFFFFFFFFFFFFFF;
    static const NavQueryId INVALID_NAV_QUERY = INVALID_NAV_MESH;

    struct NavMeshConstructionData{
        std::string meshName;
        dtNavMesh* mesh;
    };

    struct NavMeshTileData{
        unsigned char* tileData;
        int dataSize;
        //Technically these are stored in the tileData but it's useful to have them here.
        int x;
        int y;
    };
}
