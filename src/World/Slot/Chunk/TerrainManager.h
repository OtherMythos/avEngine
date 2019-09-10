#pragma once

#include <set>

namespace AV{
    class Terrain;

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

        //Destroy all the terrains created by this manager.
        void destroyTerrains();

    private:
        std::set<Terrain*> inUseTerrains;
        std::set<Terrain*> availableTerrains;
    };
}
