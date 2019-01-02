#pragma once

#include <string>

namespace AV{
    class Chunk{
    public:
        Chunk(const std::string &map, int chunkX, int chunkY);
        ~Chunk();

        bool compare(const std::string &map, int chunkX, int chunkY);

    private:
        int _chunkX, _chunkY;
        std::string _map;
    };
};
