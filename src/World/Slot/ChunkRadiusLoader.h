#pragma once

#include "World/Slot/SlotPosition.h"

#include <set>
#include <memory>

namespace AV{
    class ChunkCoordinate;
    class SlotManager;
    class Event;

    /**
    A class to determine which chunks need to be loaded or unloaded based on the player's position.
    This class does no actual loading. It only keeps track of what's loaded and what needs to be unloaded.
    It is responsible for telling the SlotManager what to do.
    */
    class ChunkRadiusLoader{
    public:
        typedef std::pair<int, int> LoadedChunkData;
        typedef std::set<LoadedChunkData> LoadedChunkContainer;

        ChunkRadiusLoader(std::shared_ptr<SlotManager> slotManager);
        /**
         Empty constructor for tests.
         */
        ChunkRadiusLoader();
        ~ChunkRadiusLoader();

        void initialise();

        bool worldEventReceiver(const Event &event);

        bool chunkLoadedInCurrentMap(int chunkX, int chunkY) const;

    private:
        std::shared_ptr<SlotManager> mSlotManager;

        bool _checkRectCircleCollision(int tileX, int tileY, int rectSize, int radius, int circleX, int circleY) const;

        void _loadChunk(const LoadedChunkData &chunk);
        void _unloadChunk(const LoadedChunkData &chunk);

        /**
        Update the position of the player.
        This will re-calculate which chunks need to be loaded and which can be unloaded.
        */
        void _updatePlayer(const SlotPosition &playerPos);

        //Internal method to update when the map changes.
        void _updateCurrentMap(const Ogre::String& oldMap, const Ogre::String& newMap);

        /**
         Check if there are any chunks and unload them. Do nothing if there are none loaded
         */
        void _unloadEverything();

        //A pair of ints containing the chunks of the current map which are currently loaded.
        LoadedChunkContainer mLoadedChunks;
    };
}
