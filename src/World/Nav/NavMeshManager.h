#pragma once

#include "NavTypes.h"
#include <vector>
#include "OgreVector3.h"
#include "DetourNavMesh.h"

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

        NavMeshId getMeshByName(const std::string& name) const;

        /**
        Generate a nav query to use with later functions.
        When no longer needed, this query must be returned with releaseNavMeshQuery.
        */
        NavQueryId generateNavQuery(NavMeshId id);

        void releaseNavMeshQuery(NavQueryId query);

        /**
        Find a path between the specified start and end points in the mesh.
        @returns 0 if succesful, -1 if not.
        */
        int queryPath(NavQueryId queryId, const Ogre::Vector3& start, const Ogre::Vector3& end, const Ogre::Vector3& extends);

        bool getNextPosition(NavQueryId queryId, const Ogre::Vector3& start, Ogre::Vector3* outVec);

        struct NavMeshData{
            bool populated;
            dtNavMesh* mesh;
            uint32 version;
        };
        static const int MAX_QUERY_POLYS = 256;
        struct NavMeshQueryData{
            dtNavMeshQuery* query;
            //The polys of the complete path.
            dtPolyRef outPath[MAX_QUERY_POLYS];
            //The number of polys in the found path
            int pathCount;
            //The number of points in the walkable path.
            int walkPathCount;
            //Vector3s of points to walk across.
            float targetWalkPath[MAX_QUERY_POLYS*3];
            //The current point in the vector list in which the object is walking. -1 if not walking.
            int currentWalkIndex;
        };

        dtNavMeshQuery* getQuery(NavQueryId id) const;

        uint32 getNumNavMeshes() const { return mNumMeshes; }

    private:
        std::vector<NavMeshData> mMeshes;
        uint32 mNumMeshes;

        bool mHoleInQueries = false;
        std::vector<NavMeshQueryData> mQueries;
    };
}
