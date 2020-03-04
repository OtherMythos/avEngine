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

        Terrain::clearShadowTexture();
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

    void* TerrainManager::requestTerrainDataPtr(uint32 width, uint32 height){
        const TerrainDataId checkingId(width, height);

        void* targetPtr = 0;
        auto it = mAvailableTerrainData.begin();
        while(it != mAvailableTerrainData.end()){
            const TerrainDataEntry& e = *it;
            if(e.first == checkingId){
                targetPtr = e.second;
                mAvailableTerrainData.erase(it);

                return targetPtr;
            }
        }

        //If we've reached this point, no data pointer exists, so we need to create one.
        assert(!targetPtr);
        targetPtr = malloc(width * height * sizeof(float));

        mInUseTerrainData.insert( {checkingId, targetPtr} );

        return targetPtr;
    }

    void TerrainManager::releaseTerrainDataPtr(void* ptr){
        for(const TerrainDataEntry& e : mInUseTerrainData){
            if(ptr == e.second){
                break;
            }
        }
    }

    void TerrainManager::getTerrainTestData(int& inUse, int& available){
        inUse = inUseTerrains.size();
        available = availableTerrains.size();
    }
}
