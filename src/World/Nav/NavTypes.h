#pragma once

#include <string>
#include "System/EnginePrerequisites.h"

class dtNavMesh;

namespace AV{
    typedef uint64 NavMeshId;
    typedef uint16 NavQueryId;

    //TODO this can't be 0, must be something higher.
    static const NavMeshId INVALID_NAV_MESH = 0;
    static const NavQueryId INVALID_NAV_QUERY = 0xFFFF;

    struct NavMeshConstructionData{
        std::string meshName;
        dtNavMesh* mesh;
    };
}
