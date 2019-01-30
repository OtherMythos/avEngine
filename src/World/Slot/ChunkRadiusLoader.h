#pragma once

#include "World/Slot/SlotPosition.h"

#include <set>
#include <memory>

namespace AV{
    class ChunkCoordinate;
    class SlotManager;

    /**
    A class to determine which chunks need to be loaded or unloaded based on the player's position.
    This class does no actual loading. It only keeps track of what's loaded and what needs to be unloaded.
    It is responsible for telling the SlotManager what to do.
    */
    class ChunkRadiusLoader{
    public:
        ChunkRadiusLoader(std::shared_ptr<SlotManager> slotManager);
        ~ChunkRadiusLoader();

        /**
        Update the position of the player.
        This will re-calculate which chunks need to be loaded and which can be unloaded.
        */
        void updatePlayer(const SlotPosition &playerPos);
        //TODO replace this with the event system when the map changes are implemented.
        void setCurrentMap();

    private:
        std::shared_ptr<SlotManager> mSlotManager;
        //TODO this radius should be pulled from the world singleton rather than being stored here.
        const int radius = 100;

        typedef std::pair<int, int> LoadedChunkData;

        bool _checkRectCircleCollision(int tileX, int tileY, int rectSize, int radius, int circleX, int circleY);

        void _loadChunk(const LoadedChunkData &chunk);
        void _unloadChunk(const LoadedChunkData &chunk);

        //A pair of ints containing the chunks of the current map which are currently loaded.
        std::set<LoadedChunkData> mLoadedChunks;
    };
}
