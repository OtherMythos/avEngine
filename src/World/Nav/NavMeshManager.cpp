#include "NavMeshManager.h"

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

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
            }
        }
        //No hole was found. Insert at the end.

        //In this case nothing else can be inserted.
        if(mMeshes.size() >= INVALID_NAV_MESH) return INVALID_NAV_MESH;
        idx = static_cast<NavMeshId>(mMeshes.size());

        mMeshes.push_back({true, mesh});

        return idx;
    }

    void NavMeshManager::unregisterNavMesh(NavMeshId id){
        //Unregistering means using an id from register nav mesh. In this case it should be valid.
        assert(id < mMeshes.size());

        memset(&(mMeshes[id]), 0, sizeof(NavMeshData));
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
                if(!mQueries[i]){
                    mQueries[i] = mNavQuery;
                    return i;
                }
            }
            //The hole should have been found so the function should have returned.
            assert(false);
        }

        mHoleInQueries = false;

        NavQueryId retVal = static_cast<NavQueryId>(mQueries.size());
        mQueries.push_back(mNavQuery);

        return retVal;
    }

    void NavMeshManager::releaseNavMeshQuery(NavQueryId query){
        assert(query < mQueries.size());
        assert(!mQueries[query]);

        mQueries[query] = 0;
        mHoleInQueries = true;
    }
}
