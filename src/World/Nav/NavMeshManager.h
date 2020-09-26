#pragma once

#include "NavTypes.h"
#include <vector>

class dtNavMesh;

namespace AV{
    class NavMeshManager{
    public:
        NavMeshManager();
        ~NavMeshManager();

        /**
        Register a nav mesh to the mesh manager.
        */
        NavMeshId registerNavMesh(dtNavMesh* mesh);

        /**
        Remove a nav mesh.
        */
        void unregisterNavMesh(NavMeshId id);

        struct NavMeshData{
            bool populated;
            dtNavMesh* mesh;
        };

    private:
        std::vector<NavMeshData> mMeshes;
    };
}