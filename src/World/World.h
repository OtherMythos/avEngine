#pragma once

#include "Logger/Log.h"
#include <memory>

namespace AV {
    class WorldSingleton;
    class SlotManager;
    
    class World{
        friend WorldSingleton;
    protected:
        World();
        ~World();
        
        std::shared_ptr<SlotManager> _slotManager;
        
    public:
        void initialise();
        
        std::shared_ptr<SlotManager> getSlotManager() { return _slotManager; };
    };
}
