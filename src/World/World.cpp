#include "World.h"

#include "Slot/SlotManager.h"

namespace AV {
    World::World(){
        _slotManager = std::make_shared<SlotManager>();
        _slotManager->initialise();
    }

    World::~World(){

    }

    SlotPosition pos = SlotPosition();
    void World::update(){
        //worldPos = SlotPosition(worldPos.chunkX, worldPos.chunkY, worldPos.position);
        pos = pos + Ogre::Vector3(1, 0, 0);
        // AV_INFO(pos.chunkX());
        // AV_INFO(pos.position());

      _slotManager->updateChunks(pos);
    }
}
