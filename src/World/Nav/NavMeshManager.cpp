#include "NavMeshManager.h"

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

#include "Logger/Log.h"
#include <cassert>
#include <cstring>

namespace AV{
    NavMeshManager::NavMeshManager(){

    }

    NavMeshManager::~NavMeshManager(){

    }

    NavMeshId NavMeshManager::registerNavMesh(dtNavMesh* mesh){
        NavMeshId idx = 0;
        for(; idx < mMeshes.size(); idx++){
            if(!mMeshes[idx].populated) {
                //A hole was found.
                mMeshes[idx].populated = true;
                mMeshes[idx].mesh = mesh;
                mMeshes[idx].version++;
            }
        }
        //No hole was found. Insert at the end.

        //In this case nothing else can be inserted.
        if(mMeshes.size() >= INVALID_NAV_MESH) return INVALID_NAV_MESH;
        idx = static_cast<NavMeshId>(mMeshes.size());

        mMeshes.push_back({true, mesh, 0});
        mNumMeshes++;

        return idx;
    }

    void NavMeshManager::unregisterNavMesh(NavMeshId id){
        //Unregistering means using an id from register nav mesh. In this case it should be valid.
        assert(id < mMeshes.size());

        memset(&(mMeshes[id]), 0, sizeof(NavMeshData));
        mNumMeshes--;
    }

    NavQueryId NavMeshManager::generateNavQuery(NavMeshId mesh){
        assert(mesh < mMeshes.size());
        const NavMeshData& foundMesh = mMeshes[mesh];
        assert(foundMesh.populated && foundMesh.mesh);

        dtNavMeshQuery* mNavQuery = dtAllocNavMeshQuery();
        dtStatus status = mNavQuery->init(foundMesh.mesh, 2048);
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

        q.query->findPath(startRef, endRef, &(startPolyPoint[0]), &(endPolyPoint[0]), &filter, q.outPath, &(q.pathCount), MAX_QUERY_POLYS);
        if(q.pathCount <= 0) return -1;

        //Find the straight path.
        int numPolys = q.pathCount;
        //if (q.outPath[numPolys-1] != m_endRef)
        //    q.query->closestPointOnPoly(q.outPath[numPolys-1], startPos, endPos, 0);


        unsigned char m_straightPathFlags[MAX_QUERY_POLYS];
        dtPolyRef m_straightPathPolys[MAX_QUERY_POLYS];
        q.query->findStraightPath(startPos, endPos, q.outPath, numPolys,
                                        q.targetWalkPath, m_straightPathFlags,
                                        m_straightPathPolys, &(q.walkPathCount), MAX_QUERY_POLYS, 0);

        return 0;
    }

    bool NavMeshManager::getNextPosition(NavQueryId queryId, const Ogre::Vector3& start, Ogre::Vector3* outVec){
        assert(queryId < mQueries.size());
        NavMeshQueryData& q = mQueries[queryId];
        assert(q.currentWalkIndex >= 0);

        if(q.currentWalkIndex >= q.walkPathCount){
            //We've reached the end of the walkable sections.
            q.currentWalkIndex = -1;
            AV_ERROR("DONE MOVING");
            return false;
        }

        float* currentGoal = &(q.targetWalkPath[q.currentWalkIndex * 3]);
        Ogre::Vector3 foundPos(*currentGoal, *(currentGoal+1), *(currentGoal+2));
        Ogre::Vector3 newPos(start);

        Ogre::Vector3 direction = foundPos - start;
        direction.normalise();
        newPos += direction * 0.01;
        Ogre::Real dist = foundPos.distance(newPos);
        AV_ERROR("distance {}", dist);
        if(dist <= 0.01){
            //It's close enough, so move to the next point.
            AV_ERROR("MOving to next");
            q.currentWalkIndex++;
        }
        *outVec = newPos;

        return true;
    }
}
