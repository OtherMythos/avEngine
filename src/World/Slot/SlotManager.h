#pragma once

#include <string>

#include <vector>
#include <set>
#include "SlotPosition.h"
#include "ChunkRadiusChecks.h"

namespace AV {
    class Chunk;

    class SlotManager{
    public:
        SlotManager();
        ~SlotManager();

        void setCurrentMap(const std::string &map);

        void loadChunk(const std::string &map, int chunkX, int chunkY);
        void unloadChunk(const std::string &map, int chunkX, int chunkY);

        void setOrigin(const SlotPosition &pos);

    private:
        std::string _currentMap = "";

        /**
         Convenience function to get the origin from the WorldSingleton.
         */
        const SlotPosition& _getOrigin();

        bool _checkIfChunkLoaded(const std::string &map, int chunkX, int chunkY);

        std::vector<Chunk*> _activeChunks;
        std::vector<Chunk*> _loadedChunks;
        std::set<Chunk*> _loadedChunksCurrentMap;
    };
}
