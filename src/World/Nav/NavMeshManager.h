#pragma once

#include "NavTypes.h"
#include <vector>
#include <map>
#include "OgreVector3.h"
#include "DetourNavMesh.h"

#include "System/Util/VersionedDataPool.h"
#include "System/Util/DataPacker.h"
#include "Event/Events/Event.h"

#include "Parser/MapNavMetaParserData.h"
#include "World/Nav/NavTypes.h"

class dtNavMesh;
class dtNavMeshQuery;

namespace AV{
    class NavMeshManager{
    private:
        struct StoredNavTileData{
            int dataSize;
            unsigned char* tileData;
            int targetMesh;
        };

    public:
        NavMeshManager();
        ~NavMeshManager();
        void initialise();

        /**
        Provide nav mesh tile data to be managed by the nav mesh manager.
        The manager will be responsible for its usage and lifetime.
        The tileData pointer should be discarded and not used after this call.
        */
        NavTilePtr yieldNavMeshTile(unsigned char* tileData, int dataSize, int targetMesh, int ownedChunkX, int ownedChunkY);

        /**
        Register a nav mesh to the mesh manager.
        */
        NavMeshId registerNavMesh(dtNavMesh* mesh, const std::string& name);

        /**
        Add a tile to the scene.
        If the parent mesh does not exist yet it will be created.
        */
        void insertNavMeshTile(NavTilePtr id);

        /**
        Remove a nav mesh tile from the scene.
        */
        void removeNavMeshTile(NavTilePtr id);

        /**
        Remove a nav mesh.
        @returns True or false depending on whether the id is valid.
        */
        bool unregisterNavMesh(NavMeshId id);

        NavMeshId getMeshByName(const std::string& name) const;

        /**
        Generate a nav query to use with later functions.
        When no longer needed, this query must be returned with releaseNavMeshQuery.
        */
        NavQueryId generateNavQuery(NavMeshId id);

        bool releaseNavMeshQuery(NavQueryId query);

        /**
        Find a path between the specified start and end points in the mesh.
        @returns 0 if succesful, -1 if not.
        */
        int queryPath(NavQueryId queryId, const Ogre::Vector3& start, const Ogre::Vector3& end, const Ogre::Vector3& extends);

        bool getNextPosition(NavQueryId queryId, const Ogre::Vector3& start, Ogre::Vector3* outVec, float speed);

        static const int MAX_QUERY_POLYS = 256;
        struct NavMeshQueryData{
            dtNavMeshQuery* query;
            NavMeshId targetMesh;
            //TODO this could be separated off somewhere else if not being used.
            struct{
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
            }i;
        };

        uint32 getNumNavMeshes() const { return mNumMeshes; }
        uint32 getNumQueries() const { return mNumQueries; }
        bool isNavMeshIdValid(NavMeshId id) const { return mMeshes.isIdValid(id); }
        bool isNavMeshQueryValid(NavQueryId id) const;

        dtNavMeshQuery* getQuery(NavQueryId id);

    private:
        struct NavMeshDataEntry{
            dtNavMesh* target;
            uint8 numTiles;
        };
        VersionedDataPool<NavMeshDataEntry> mMeshes;
        DataPacker<StoredNavTileData> mStoredTiles;
        std::map<std::string, NavMeshId> mMeshesMap;
        std::vector<MapNavMetaParserData> mMapData;
        uint32 mNumMeshes;
        uint32 mNumQueries;

        /**
        Get pointer data from a query, checking at the same time that the query and the mesh are valid.
        @returns 0 if either the query or mesh are invalid.
        */
        NavMeshQueryData* _getQueryData(NavQueryId queryId);

        /**
        Reset the values of a query for a new path.
        */
        void resetQuery(NavMeshQueryData* q);

        void _processMapChange(const std::string& mapName);

        bool worldEventReceiver(const Event &e);

        static void _destroyNavMeshTile(void* tile);

        bool mHoleInQueries = false;
        VersionedDataPool<NavMeshQueryData> mQueries;

    public:
        uint32 getNumDefinedMaps() const { return static_cast<int>(mMapData.size()); }
        int getNumTilesForMesh(NavMeshId mesh) const;
    };
}
