#include "World.h"

#include "Slot/SlotManager.h"

namespace AV {
    World::World(){
        _slotManager = std::make_shared<SlotManager>();
        _slotManager->initialise();

        initialise();
    }

    World::~World(){

    }

    void World::initialise(){
        // _slotManager->setOrigin(SlotPosition());
        // _slotManager->setCurrentMap("Map");
        // _slotManager->loadChunk("Map", 0, 0);
        // _slotManager->loadChunk("Map", 1, 0);

        //_slotManager->unloadChunk("Map", 0, 0);
        //_slotManager->unloadChunk("Map", 1, 0);
    }
}
