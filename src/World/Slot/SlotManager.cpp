#include "SlotManager.h"

#include "World/WorldSingleton.h"

#include "Chunk/Chunk.h"

namespace AV {
    SlotManager::SlotManager(){

    }

    SlotManager::~SlotManager(){

    }

    void SlotManager::setCurrentMap(const std::string &map){
        _currentMap = map;
    }

    bool SlotManager::loadChunk(const std::string &map, int chunkX, int chunkY){
        if(chunkX < 0 || chunkY < 0) return false;
        if(_checkIfChunkLoaded(map, chunkX, chunkY)) return false;

        Chunk* chunk = new Chunk(map, chunkX, chunkY);
        if(ChunkRadiusChecks::isChunkWithinOrigin(chunkX, chunkY)){
            if(map == _currentMap){
                _activeChunks.push_back(chunk);
            }else{
                _loadedChunks.push_back(chunk);
            }
        }else{
            if(map == _currentMap){
                _loadedChunksCurrentMap.insert(chunk);
            }
            _loadedChunks.push_back(chunk);
        }
        
        return true;
    }

    void SlotManager::unloadChunk(const std::string &map, int chunkX, int chunkY){

    }

    void SlotManager::setOrigin(const SlotPosition &pos){
        if(_getOrigin() == pos) return;

        WorldSingleton::_origin = pos;
    }

    const SlotPosition& SlotManager::_getOrigin(){
        return WorldSingleton::getOrigin();
    }

    bool SlotManager::_checkIfChunkLoaded(const std::string &map, int chunkX, int chunkY){
        for(Chunk *c : _activeChunks){
            if(c->compare(map, chunkX, chunkY))
                return true;
        }
        for(Chunk *c : _loadedChunks){
            if(c->compare(map, chunkX, chunkY))
                return true;
        }
        return false;
    }
}
