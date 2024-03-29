#pragma once

#include <set>

namespace AV{
    class Terrain;

    typedef unsigned int uint32;

    /**
    A class to manage and share terrain resources between chunks.
    Rather than creating and destroying a terrain instance each time a chunk is loaded or unloaded, this class loads them once and then pools them together.
    Terrains can be requested and released for use at a later date.
    */
    class TerrainManager{
    public:
        TerrainManager();
        ~TerrainManager();

        //Request a terrain instance. When done with you should call releaseTerrain().
        Terrain* requestTerrain();

        //Release a terrain pointer. This will allow this specific pointer to be used again by another chunk.
        void releaseTerrain(Terrain* t);

        //Destroy all the terrains created by this manager. These are ogre objects so this should be called during world shutdown.
        void destroyTerrains();
        //Destroy the malloced terrain height data. This should be called during engine shutdown.
        void destroyTerrainData();

        void getTerrainTestData(int& inUse, int& available);

        /**
        Request a pointer to a piece of data to store terrain height values.
        These values can be used for the physics simulation.
        The system will recycle these pieces of memory as much as possible.
        The size of the buffer will be width * height * sizeof(float).
        */
        void* requestTerrainDataPtr(uint32 width, uint32 height);
        void releaseTerrainDataPtr(void* ptr);

    private:
        std::set<Terrain*> inUseTerrains;
        std::set<Terrain*> availableTerrains;

        typedef std::pair<uint32, uint32> TerrainDataId;
        typedef std::pair<TerrainDataId, void*> TerrainDataEntry;

        std::set<TerrainDataEntry> mAvailableTerrainData;
        std::set<TerrainDataEntry> mInUseTerrainData;
    };
}
