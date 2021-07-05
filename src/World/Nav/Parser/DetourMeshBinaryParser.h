#pragma once

#include <vector>
#include <string>

#include "World/Nav/NavTypes.h"

class dtNavMesh;

namespace AV{

    /**
    A utility class to parse a detour mesh serialised to a file.
    */
    class DetourMeshBinaryParser{
    public:
        DetourMeshBinaryParser();

        dtNavMesh* parseFile(const std::string& filePath);

        /**
        Parse a json nav mesh file.
        @returns true or false depending on whether the parse was succesful.
        */
        bool parseJsonMetaFile(const std::string& filePath, std::vector<NavMeshConstructionData>& outVec);

    private:
        dtNavMesh* _loadAll(const char* path) const;
    };
}
