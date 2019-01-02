#include "Chunk.h"

namespace AV{
    Chunk::Chunk(const std::string &map, int chunkX, int chunkY)
    : _map(map),
    _chunkX(chunkX),
    _chunkY(chunkY){

    }

    Chunk::~Chunk(){

    }

    bool Chunk::compare(const std::string &map, int chunkX, int chunkY){
        if(chunkX == _chunkX && chunkY == _chunkY && map == _map) return true;
        else return false;
    }
};
