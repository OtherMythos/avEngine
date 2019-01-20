#pragma once

#include "World/Slot/SlotPosition.h"

#include <set>
#include <memory>

namespace AV{
    class ChunkCoordinate;
    class SlotManager;

    class ChunkRadiusLoader{
    public:
        ChunkRadiusLoader(std::shared_ptr<SlotManager> slotManager);
        ~ChunkRadiusLoader();

        void updatePlayer(const SlotPosition &playerPos);
        void setCurrentMap();

    private:
        std::shared_ptr<SlotManager> mSlotManager;
        const int radius = 100;
        //SlotPosition centrePos;

        typedef std::pair<int, int> LoadedChunkData;

        bool _checkRectCircleCollision(int tileX, int tileY, int rectSize, int radius, int circleX, int circleY);

        void _loadChunk(const LoadedChunkData &chunk);
        void _unloadChunk(const LoadedChunkData &chunk);

        std::set<LoadedChunkData> mLoadedChunks;
    };
}
