#pragma once

#include "NavTypes.h"
#include <vector>

class dtNavMesh;
class dtNavMeshQuery;

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

        /**
        Generate a nav query to use with later functions.
        When no longer needed, this query must be returned with releaseNavMeshQuery.
        */
        NavQueryId generateNavQuery(NavMeshId id);

        void releaseNavMeshQuery(NavQueryId query);

        struct NavMeshData{
            bool populated;
            dtNavMesh* mesh;
        };

        dtNavMeshQuery* getQuery(NavQueryId id) const;

    private:
        std::vector<NavMeshData> mMeshes;

        bool mHoleInQueries = false;
        std::vector<dtNavMeshQuery*> mQueries;
    };
}
