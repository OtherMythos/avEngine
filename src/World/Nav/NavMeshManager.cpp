#include "NavMeshManager.h"

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
}