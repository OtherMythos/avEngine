#pragma once

#include <string>

class dtNavMesh;

namespace AV{

    /**
    A utility class to parse a detour mesh serialised to a file.
    */
    class DetourMeshBinaryParser{
    public:
        DetourMeshBinaryParser();

        dtNavMesh* parseFile(const std::string& filePath);

    private:
        dtNavMesh* _loadAll(const char* path) const;
    };
}