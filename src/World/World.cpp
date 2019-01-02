#include "World.h"

#include "Slot/SlotManager.h"

namespace AV {
    World::World(){
        _slotManager = std::make_shared<SlotManager>();
        initialise();
    }
    
    World::~World(){
        
    }
    
    void World::initialise(){
        _slotManager->setOrigin(SlotPosition());
    }
}
