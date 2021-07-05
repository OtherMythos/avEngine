#include "NavMeshManager.h"

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

#include "Logger/Log.h"
#include <cassert>
#include <cstring>

namespace AV{
    NavMeshManager::NavMeshManager()
        : mNumMeshes(0) {

    }

    NavMeshManager::~NavMeshManager(){

    }

    NavMeshId NavMeshManager::registerNavMesh(dtNavMesh* mesh, const std::string& name){
        NavMeshId idx = 0;
        for(; idx < mMeshes.size(); idx++){
            if(!mMeshes[idx].populated) {
                //A hole was found.
                mMeshes[idx].populated = true;
                mMeshes[idx].mesh = mesh;
                //Version was incremented during destruction.
                NavMeshId retId = _valuesToMeshId(idx, mMeshes[idx].version);
                mMeshesMap[name] = retId;
                mNumMeshes++;
                return retId;
            }
        }
        //No hole was found. Insert at the end.

        idx = static_cast<NavMeshId>(mMeshes.size());

        //Versions start at 1 so 0 can mean invalid.
        const uint32 targetVersion = 1;
        mMeshes.push_back({true, mesh, targetVersion});
        mNumMeshes++;

        NavMeshId retId = _valuesToMeshId(idx, targetVersion);
        mMeshesMap[name] = retId;
        return retId;
    }

    bool NavMeshManager::unregisterNavMesh(NavMeshId id){
        //Unregistering means using an id from register nav mesh. In this case it should be valid.
        if(id == INVALID_NAV_MESH) return false;

        NavMeshData* foundMesh = _getMeshByHandle(id);
        if(!foundMesh) return INVALID_NAV_QUERY;

        foundMesh->mesh = 0;
        //TODO proper deletion here.
        foundMesh->populated = false;
        foundMesh->version++;
        assert(mNumMeshes > 0);
        mNumMeshes--;

        //TODO remove from the map here.

        return true;
    }

    void NavMeshManager::_meshIdToValues(NavMeshId id, uint32* idx, uint32* version) const{
        uint32 outIdx = static_cast<uint32>( (id >> 32) & 0xFFFFFF );

        *idx = outIdx;
        *version = static_cast<uint32>(id);
    }

    NavMeshId NavMeshManager::_valuesToMeshId(uint32 idx, uint32 version) const{
        return (AV::NavMeshId(idx) << 32) + version;
    }

    NavMeshId NavMeshManager::getMeshByName(const std::string& name) const{
        auto it = mMeshesMap.find(name);
        if(it == mMeshesMap.end()) return INVALID_NAV_MESH;

        return it->second;
    }

    NavMeshManager::NavMeshData* NavMeshManager::_getMeshByHandle(NavMeshId mesh){
        if(mesh == INVALID_NAV_MESH) return 0;
        uint32 idx, version;
        _meshIdToValues(mesh, &idx, &version);
        if(idx >= mMeshes.size()) return 0;

        if(mMeshes[idx].version != version) return 0;

        return &(mMeshes[idx]);
    }

    NavQueryId NavMeshManager::generateNavQuery(NavMeshId mesh){
        NavMeshData* foundMesh = _getMeshByHandle(mesh);
        if(!foundMesh) return INVALID_NAV_QUERY;
        assert(foundMesh->populated && foundMesh->mesh);

        dtNavMeshQuery* mNavQuery = dtAllocNavMeshQuery();
        dtStatus status = mNavQuery->init(foundMesh->mesh, 2048);
        if(dtStatusFailed(status)){
            return INVALID_NAV_QUERY;
        }

        if(mHoleInQueries){
            for(NavQueryId i = 0; i < mQueries.size(); i++){
                if(!mQueries[i].query){
                    memset(&(mQueries[i]), 0, sizeof(NavMeshQueryData));
                    mQueries[i].query = mNavQuery;
                    return i;
                }
            }
            //The hole should have been found so the function should have returned.
            assert(false);
        }

        mHoleInQueries = false;

        NavQueryId retVal = static_cast<NavQueryId>(mQueries.size());
        NavMeshQueryData queryData;
        memset(&queryData, 0, sizeof(NavMeshQueryData));
        queryData.query = mNavQuery;
        mQueries.push_back(queryData);

        return retVal;
    }

    void NavMeshManager::releaseNavMeshQuery(NavQueryId query){
        assert(query < mQueries.size());
        assert(!mQueries[query].query);

        //TODO destroy the query ptr.
        memset(&(mQueries[query]), 0, sizeof(NavMeshQueryData));
        mHoleInQueries = true;
    }

    dtNavMeshQuery* NavMeshManager::getQuery(NavQueryId id) const{
        assert(id < mQueries.size());
        return mQueries[id].query;
    }

    int NavMeshManager::queryPath(NavQueryId queryId, const Ogre::Vector3& start, const Ogre::Vector3& end, const Ogre::Vector3& extends){
        assert(queryId < mQueries.size());
        NavMeshQueryData& q = mQueries[queryId];

        //Find the start and end pos.
        const float startPos[3] = {start.x, start.y, start.z};
        const float endPos[3] = {end.x, end.y, end.z};
        float startPolyPoint[3];
        float endPolyPoint[3];
        dtPolyRef startRef;
        dtPolyRef endRef;

        static const float extent[3] = {extends.x, extends.y, extends.z};
        dtQueryFilter filter;

        dtStatus result1 = q.query->findNearestPoly(startPos, extent, &filter, &startRef, &(startPolyPoint[0]));
        dtStatus result2 = q.query->findNearestPoly(endPos, extent, &filter, &endRef, &(endPolyPoint[0]));

        assert(result1 == DT_SUCCESS && result2 == DT_SUCCESS);

        q.query->findPath(startRef, endRef, &(startPolyPoint[0]), &(endPolyPoint[0]), &filter, q.i.outPath, &(q.i.pathCount), MAX_QUERY_POLYS);
        if(q.i.pathCount <= 0) return -1;

        //Find the straight path.
        int numPolys = q.i.pathCount;
        //if (q.outPath[numPolys-1] != m_endRef)
        //    q.query->closestPointOnPoly(q.outPath[numPolys-1], startPos, endPos, 0);


        unsigned char m_straightPathFlags[MAX_QUERY_POLYS];
        dtPolyRef m_straightPathPolys[MAX_QUERY_POLYS];
        q.query->findStraightPath(startPos, endPos, q.i.outPath, numPolys,
                                        q.i.targetWalkPath, m_straightPathFlags,
                                        m_straightPathPolys, &(q.i.walkPathCount), MAX_QUERY_POLYS, 0);

        return 0;
    }

    void NavMeshManager::resetQuery(NavMeshQueryData* q){
        memset(&(q->i), 0, sizeof(NavMeshQueryData::i));
    }

    bool NavMeshManager::getNextPosition(NavQueryId queryId, const Ogre::Vector3& start, Ogre::Vector3* outVec, float speed){
        assert(queryId < mQueries.size());
        NavMeshQueryData& q = mQueries[queryId];
        assert(q.i.currentWalkIndex >= 0);

        if(q.i.currentWalkIndex >= q.i.walkPathCount){
            //We've reached the end of the walkable sections.
            //q.i.currentWalkIndex = -1;
            AV_ERROR("DONE MOVING");
            //Reset the query.
            resetQuery(&q);
            return false;
        }

        //OPTIMISATION could cache some of these values, for instance the distance and direction.
        float* currentGoal = &(q.i.targetWalkPath[q.i.currentWalkIndex * 3]);
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
            q.i.currentWalkIndex++;
            *outVec = foundPos;
            return true;
        }
        *outVec = newPos;

        return true;
    }
}
