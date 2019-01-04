#pragma once

#include <string>

namespace AV{
    
    /**
     A chunk is a piece of map inserted into a slot in the SlotManager.
     Chunks contain resources such as meshes and physics shapes.
     */
    class Chunk{
    public:
        Chunk(const std::string &map, int chunkX, int chunkY);
        ~Chunk();

        /**
         Compare this chunk to another.
         
         @return
         True if this chunk has the same coordinates as passed in. False if not.
         */
        bool compare(const std::string &map, int chunkX, int chunkY);

    private:
        int _chunkX, _chunkY;
        std::string _map;
    };
};
