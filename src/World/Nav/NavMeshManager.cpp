#include "NavMeshManager.h"

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

#include "Logger/Log.h"
#include <cassert>
#include <cstring>

#include "Parser/MapNavMetaParser.h"
#include "System/SystemSetup/SystemSettings.h"
#include "System/Util/DataPacker.h"

#include "Event/Events/WorldEvent.h"
#include "Event/EventDispatcher.h"

#include "World/WorldSingleton.h"

namespace AV{
    NavMeshManager::NavMeshManager()
        : mNumMeshes(0) {

    }

    NavMeshManager::~NavMeshManager(){
        EventDispatcher::unsubscribe(EventType::World, this);
    }

    //TODO I could consider splitting just the static parts out somewhere else.
    static NavMeshManager* _current;

    void NavMeshManager::initialise(){
        EventDispatcher::subscribe(EventType::World, AV_BIND(NavMeshManager::worldEventReceiver));

        //The map is set initially so it would miss the event.
        _processMapChange(WorldSingleton::getCurrentMap());

        _current = this;
    }

    NavTileId NavMeshManager::yieldNavMeshTile(unsigned char* tileData, int dataSize, int targetMesh){
        void* value = mStoredTiles.storeEntry({dataSize, tileData, targetMesh});

        NavTileId sharedPtr = NavTileId(value, _destroyNavMeshTile);

        return sharedPtr;
    }

    void NavMeshManager::_destroyNavMeshTile(void* tile){
        StoredNavTileData& entry = _current->mStoredTiles.getEntry(tile);

    }

    //TODO might want to rename navTileId to be a ptr something or other.
    void NavMeshManager::insertNavMeshTile(NavTileId id){
        const StoredNavTileData& foundData = mStoredTiles.getEntry(id.get());
        const MapNavMetaParserData& navData = mMapData[foundData.targetMesh];

        NavMeshId result = getMeshByName(navData.meshName);
        if(result == INVALID_NAV_MESH){
            //Create the mesh if it does not already exist.
            dtNavMesh* mesh = dtAllocNavMesh();
            if (!mesh){
                assert(false);
            }

            float m_tileSize = navData.tileSize;
            float m_cellSize = navData.cellSize;
            dtNavMeshParams params;
            params.tileWidth = m_tileSize*m_cellSize;
            params.tileHeight = m_tileSize*m_cellSize;
            params.maxTiles = 32;
            params.maxPolys = 1000;
            params.orig[0] = 0.0f;
            params.orig[1] = 0.0f;
            params.orig[2] = 0.0f;

            dtStatus status = mesh->init(&params);
            if (dtStatusFailed(status))
            {
                assert(false);
            }

            result = registerNavMesh(mesh, navData.meshName);
        }
        assert(result != INVALID_NAV_MESH);

        dtNavMesh* foundMesh = mMeshes.getEntry(result);
        dtMeshHeader* h = (dtMeshHeader*)foundData.tileData;
        foundMesh->removeTile(foundMesh->getTileRefAt(h->x,h->y,0),0,0);
        dtStatus addResult = foundMesh->addTile(foundData.tileData, foundData.dataSize, 0, 0, 0);
        assert(!dtStatusFailed(addResult));
    }

    NavMeshId NavMeshManager::registerNavMesh(dtNavMesh* mesh, const std::string& name){
        uint64 storedId = mMeshes.storeEntry(mesh);
        mNumMeshes++;
        mMeshesMap[name] = storedId;

        return storedId;
    }

    bool NavMeshManager::unregisterNavMesh(NavMeshId id){
        bool result = mMeshes.removeEntry(id);
        if(!result) return false;
        assert(mNumMeshes > 0);
        mNumMeshes--;

        return true;
    }

    NavMeshId NavMeshManager::getMeshByName(const std::string& name) const{
        auto it = mMeshesMap.find(name);
        if(it == mMeshesMap.end()) return INVALID_NAV_MESH;

        return it->second;
    }

    NavQueryId NavMeshManager::generateNavQuery(NavMeshId mesh){
        if(!mMeshes.isIdValid(mesh)) return INVALID_NAV_QUERY;

        dtNavMesh* foundMesh = mMeshes.getEntry(mesh);
        dtNavMeshQuery* mNavQuery = dtAllocNavMeshQuery();
        dtStatus status = mNavQuery->init(foundMesh, 2048);
        if(dtStatusFailed(status)){
            return INVALID_NAV_QUERY;
        }

        NavMeshQueryData queryData;
        memset(&queryData, 0, sizeof(NavMeshQueryData));
        queryData.query = mNavQuery;
        queryData.targetMesh = mesh;
        NavQueryId retVal = mQueries.storeEntry(queryData);
        mNumQueries++;

        return retVal;
    }

    bool NavMeshManager::releaseNavMeshQuery(NavQueryId query){
        NavMeshQueryData* navEntry = mQueries.getEntryCheck(query);
        if(!navEntry) return false;

        dtFreeNavMeshQuery(navEntry->query);
        mQueries.removeEntry(query);

        assert(mNumQueries > 0);
        mNumQueries--;

        return true;
    }

    bool NavMeshManager::isNavMeshQueryValid(NavQueryId id) const{
        if(!mQueries.isIdValid(id)) return false;

        const NavMeshQueryData& queryData = mQueries.getEntry(id);
        if(!mMeshes.isIdValid(queryData.targetMesh)) return false;

        return true;
    }

    dtNavMeshQuery* NavMeshManager::getQuery(NavQueryId id){
        NavMeshManager::NavMeshQueryData* result = NavMeshManager::_getQueryData(id);
        return result->query;
    }

    NavMeshManager::NavMeshQueryData* NavMeshManager::_getQueryData(NavQueryId queryId){
        if(!mQueries.isIdValid(queryId)) return 0;

        NavMeshQueryData& queryData = mQueries.getEntry(queryId);
        if(!mMeshes.isIdValid(queryData.targetMesh)) return 0;

        return &queryData;
    }

    int NavMeshManager::queryPath(NavQueryId queryId, const Ogre::Vector3& start, const Ogre::Vector3& end, const Ogre::Vector3& extends){
        NavMeshQueryData* q = _getQueryData(queryId);
        if(!q) return -1;

        //Find the start and end pos.
        const float startPos[3] = {start.x, start.y, start.z};
        const float endPos[3] = {end.x, end.y, end.z};
        float startPolyPoint[3];
        float endPolyPoint[3];
        dtPolyRef startRef;
        dtPolyRef endRef;

        static const float extent[3] = {extends.x, extends.y, extends.z};
        dtQueryFilter filter;

        dtStatus result1 = q->query->findNearestPoly(startPos, extent, &filter, &startRef, &(startPolyPoint[0]));
        dtStatus result2 = q->query->findNearestPoly(endPos, extent, &filter, &endRef, &(endPolyPoint[0]));

        assert(result1 == DT_SUCCESS && result2 == DT_SUCCESS);

        q->query->findPath(startRef, endRef, &(startPolyPoint[0]), &(endPolyPoint[0]), &filter, q->i.outPath, &(q->i.pathCount), MAX_QUERY_POLYS);
        if(q->i.pathCount <= 0) return -1;

        //Find the straight path.
        int numPolys = q->i.pathCount;
        //if (q->outPath[numPolys-1] != m_endRef)
        //    q->query->closestPointOnPoly(q->outPath[numPolys-1], startPos, endPos, 0);


        unsigned char m_straightPathFlags[MAX_QUERY_POLYS];
        dtPolyRef m_straightPathPolys[MAX_QUERY_POLYS];
        q->query->findStraightPath(startPos, endPos, q->i.outPath, numPolys,
                                        q->i.targetWalkPath, m_straightPathFlags,
                                        m_straightPathPolys, &(q->i.walkPathCount), MAX_QUERY_POLYS, 0);

        return 0;
    }

    void NavMeshManager::resetQuery(NavMeshQueryData* q){
        memset(&(q->i), 0, sizeof(NavMeshQueryData::i));
    }

    bool NavMeshManager::getNextPosition(NavQueryId queryId, const Ogre::Vector3& start, Ogre::Vector3* outVec, float speed){
        NavMeshQueryData* q = _getQueryData(queryId);
        if(!q) return false;
        assert(q->i.currentWalkIndex >= 0);

        if(q->i.currentWalkIndex >= q->i.walkPathCount){
            //We've reached the end of the walkable sections.
            //q->i.currentWalkIndex = -1;
            AV_ERROR("DONE MOVING");
            //Reset the query.
            resetQuery(q);
            return false;
        }

        //OPTIMISATION could cache some of these values, for instance the distance and direction.
        float* currentGoal = &(q->i.targetWalkPath[q->i.currentWalkIndex * 3]);
        Ogre::Vector3 foundPos(*currentGoal, *(currentGoal+1), *(currentGoal+2));
        Ogre::Vector3 newPos(start);

        Ogre::Vector3 direction = foundPos - start;
        direction.normalise();

        newPos += direction * speed;
        Ogre::Real dist = foundPos.distance(newPos);
        //AV_ERROR("distance {}", dist);
        if(dist <= speed){
            //It's close enough, so move to the next point.
            AV_ERROR("Moving to next");
            q->i.currentWalkIndex++;
            *outVec = foundPos;
            return true;
        }
        *outVec = newPos;

        return true;
    }

    void NavMeshManager::_processMapChange(const std::string& mapName){
        mMapData.clear();
        if(!SystemSettings::isMapsDirectoryViable()) {
            return;
        }

        std::string filePath = SystemSettings::getMapsDirectory() + "/" + mapName + "/nav.json";

        MapNavMetaParser p;
        p.parseFile(filePath, mMapData);
    }

    bool NavMeshManager::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        if(event.eventId() == EventId::WorldMapChange){
            const WorldEventMapChange& wEvent = (WorldEventMapChange&)event;
            _processMapChange(wEvent.newMapName);
        }
        return true;
    }
}
