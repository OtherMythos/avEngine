#include "TerrainManager.h"

#include "Terrain/Terrain.h"

namespace AV{
    TerrainManager::TerrainManager(){

    }

    TerrainManager::~TerrainManager(){
        destroyTerrainData();
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

    void TerrainManager::destroyTerrainData(){
        //This function should only be called when the engine is shutting down.
        assert(mInUseTerrainData.size() == 0);
        for(const TerrainDataEntry& e : mAvailableTerrainData){
            float* targetPtr = (float*)e.second;
            delete[] targetPtr;
        }
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
                mInUseTerrainData.insert(*it);
                mAvailableTerrainData.erase(it);

                return targetPtr;
            }
        }

        //If we've reached this point, no data pointer exists, so we need to create one.
        assert(!targetPtr);
        //targetPtr = malloc(width * height * sizeof(float));
        targetPtr = new float[width * height];

        mInUseTerrainData.insert( {checkingId, targetPtr} );

        return targetPtr;
    }

    void TerrainManager::releaseTerrainDataPtr(void* ptr){
        for(auto it = mInUseTerrainData.begin(); it != mInUseTerrainData.end(); it++){
            if((*it).second == ptr){
                mAvailableTerrainData.insert( *it );
                mInUseTerrainData.erase(it);
                return;
            }
        }
        assert(false); //It should find something in the list.
    }

    void TerrainManager::getTerrainTestData(int& inUse, int& available){
        inUse = inUseTerrains.size();
        available = availableTerrains.size();
    }
}
