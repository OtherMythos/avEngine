#pragma once

#include <vector>
#include <string>

#include "World/Nav/NavTypes.h"

class dtNavMesh;
class dtNavMeshTile;

namespace AV{

    /**
    A utility class to parse a detour mesh serialised to a file.
    */
    class DetourMeshBinaryParser{
    public:
        DetourMeshBinaryParser();

        dtNavMesh* parseFile(const std::string& filePath);

        struct TileData{
            unsigned char* tileData;
            int dataSize;

            int x;
            int y;
        };
        bool parseTile(const std::string& filePath, TileData* out) const;

        /**
        Parse a json nav mesh file.
        @returns true or false depending on whether the parse was succesful.
        */
        bool parseJsonMetaFile(const std::string& filePath, std::vector<NavMeshTileData>& outVec) const;

    private:
        dtNavMesh* _loadAll(const char* path) const;
    };
}
