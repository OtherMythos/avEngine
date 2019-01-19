#include "World.h"

#include "Slot/SlotPosition.h"
#include "Slot/SlotManager.h"
#include "Slot/ChunkCoordinate.h"

namespace AV {
    World::World(){
        _slotManager = std::make_shared<SlotManager>();
        _slotManager->initialise();

        //_slotManager->loadChunk(ChunkCoordinate(0, 0, "map"));
        // _slotManager->activateChunk(ChunkCoordinate(0, 0, "map"));
        // _slotManager->constructChunk(ChunkCoordinate(1, 0, "map"));
        // _slotManager->constructChunk(ChunkCoordinate(1, 0, "map"));
        // _slotManager->activateChunk(ChunkCoordinate(1, 0, "map"));
        _slotManager->activateChunk(ChunkCoordinate(0, 0, "map"));
        _slotManager->deActivateChunk(ChunkCoordinate(0, 0, "map"));
        _slotManager->activateChunk(ChunkCoordinate(0, 0, "map"));
        _slotManager->destroyChunk(ChunkCoordinate(0, 0, "map"));

        //_slotManager->deActivateChunk(ChunkCoordinate(0, 0, "map"));
        //_slotManager->constructChunk(ChunkCoordinate(0, 0, "map"));
    }

    World::~World(){

    }

    void World::update(){
        _slotManager->update();
    }
}
