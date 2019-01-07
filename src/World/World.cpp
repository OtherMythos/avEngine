#include "World.h"

#include "Slot/SlotManager.h"

namespace AV {
    World::World(){
        _slotManager = std::make_shared<SlotManager>();
        _slotManager->initialise();
    }

    World::~World(){

    }

    //Ew, very hacky. Clean this up when you get the time.
    SlotPosition pos = SlotPosition();
    Ogre::Vector3 ogrePos(0, 0, 0);
    int direction = 1;
    void World::update(){
        //worldPos = SlotPosition(worldPos.chunkX, worldPos.chunkY, worldPos.position);
        //pos = pos + Ogre::Vector3(1 * direction, 0, 0);
        ogrePos += Ogre::Vector3(1 * direction, 0, 0);
        if(ogrePos.x > 300) direction = -1;
        if(ogrePos.x < -300) direction = 1;
        pos = SlotPosition(ogrePos);
        // AV_INFO(pos.chunkX());
        // AV_INFO(pos.position());

      _slotManager->updateChunks(pos);
    }
}
