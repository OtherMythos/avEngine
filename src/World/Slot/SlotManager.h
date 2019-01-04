#pragma once

#include <string>

#include <vector>
#include <set>
#include "SlotPosition.h"
#include "ChunkRadiusChecks.h"

namespace Ogre{
    class SceneNode;
}

namespace AV {
    class Chunk;

    /**
     The SlotManager component manages the chunk and slot system in the world.
     It provides the open world style level streaming used by the engine.
     */
    class SlotManager{
    public:
        SlotManager();
        ~SlotManager();

        /**
         Set the current map of the world.
         
         @param map
         The name of the map to set.
         */
        void setCurrentMap(const std::string &map);

        /**
         Load a chunk of a map.
         
         @remarks
         If the chunk of that map hasn't already been created, it will be created and stored in a list depending on whether or not the map it belongs to is the current map.
         The user can use this function to pre-load chunks which aren't a part of the current map, in which case they will be loaded in the background.
         */
        bool loadChunk(const std::string &map, int chunkX, int chunkY);
        
        /**
         Unload a chunk of a map.
         
         @return
         True if the chunk was unloaded. False if not.
         */
        bool unloadChunk(const std::string &map, int chunkX, int chunkY);

        /**
         Set the origin position of the world.
         
         @remarks
         This function will perform a complete origin shift, where many objects such as ogre meshes, bullet shapes and entities are shifted back in relation to the origin.
         Ultimately the purpose of this function is to prevent floating point issues by moving objects closer to the origin when necessary.
         */
        void setOrigin(const SlotPosition &pos);

    private:
        /**
         The name of the current map.
         */
        std::string _currentMap = "";

        /**
         Convenience function to get the origin from the WorldSingleton.
         */
        const SlotPosition& _getOrigin();

        /**
         Check if a chunk coordinate is contained within any of the chunk data structures.
         
         @return
         True if the chunk is contained within any of chunk data structures (meaning it is in some way loaded).
         False if its not present in any.
         */
        bool _checkIfChunkLoaded(const std::string &map, int chunkX, int chunkY);

        std::vector<Chunk*> _activeChunks;
        std::vector<Chunk*> _loadedChunks;
        std::set<Chunk*> _loadedChunksCurrentMap;
        
        Ogre::SceneNode* _parentSlotNode;
    };
}
