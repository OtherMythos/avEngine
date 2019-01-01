#pragma once

#include <string>

#include "SlotPosition.h"

namespace AV {
    class SlotManager{
    public:
        SlotManager();
        ~SlotManager();
        
        void setCurrentMap(const std::string &map);
        
        void loadChunk(const std::string &map, int chunkX, int chunkY);
        void unloadChunk(const std::string &map, int chunkX, int chunkY);
        
        void setOrigin(const SlotPosition &pos);
        
    private:
        std::string _currentMap;
        SlotPosition _origin;
    };
}
