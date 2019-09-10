#include "TerrainManager.h"

#include "Terrain/Terrain.h"

namespace AV{
    TerrainManager::TerrainManager(){

    }

    TerrainManager::~TerrainManager(){
        destroyTerrains();
    }

    void TerrainManager::destroyTerrains(){
        for(Terrain* t : inUseTerrains){
            delete t;
        }
        for(Terrain* t : availableTerrains){
            delete t;
        }
        inUseTerrains.clear();
        availableTerrains.clear();
    }

    Terrain* TerrainManager::requestTerrain(){
        if(availableTerrains.size() > 0){
            auto i = availableTerrains.begin(); //Get a pointer from the list.

            Terrain* retTerr = *i;
            availableTerrains.erase(i);
            inUseTerrains.insert(retTerr);
            return retTerr;
        }

        //There are no available terrains, or they're all in use. We need to construct a new one.
        Terrain* terr = new Terrain();
        inUseTerrains.insert(terr);

        return terr;
    }

    void TerrainManager::releaseTerrain(Terrain* t){
        auto i = inUseTerrains.find(t);
        if(i != inUseTerrains.end()){
            inUseTerrains.erase(i);
            availableTerrains.insert(t);
        }
    }
}
