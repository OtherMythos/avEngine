#include "SlotManager.h"

namespace AV {
    SlotManager::SlotManager(){
        
    }
    
    SlotManager::~SlotManager(){
        
    }
    
    void SlotManager::setCurrentMap(const std::string &map){
        _currentMap = map;
    }
    
    void SlotManager::loadChunk(const std::string &map, int chunkX, int chunkY){
        
    }
    
    void SlotManager::unloadChunk(const std::string &map, int chunkX, int chunkY){
        
    }
    
    void SlotManager::setOrigin(const SlotPosition &pos){
        if(_origin == pos) return;
        
        _origin = pos;
    }
}
