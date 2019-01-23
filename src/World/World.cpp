#include "World.h"

#include "Slot/SlotPosition.h"
#include "Slot/SlotManager.h"
#include "Slot/ChunkCoordinate.h"
#include "Slot/ChunkRadiusLoader.h"

#include "OgreCamera.h"

namespace AV {
    World::World(){
        _slotManager = std::make_shared<SlotManager>();
        mChunkRadiusLoader = std::make_shared<ChunkRadiusLoader>(_slotManager);
        _slotManager->initialise();

        _slotManager->setCurrentMap("map");

        //_slotManager->loadChunk(ChunkCoordinate(0, 0, "map"));
        // _slotManager->activateChunk(ChunkCoordinate(0, 0, "map"));
        // _slotManager->constructChunk(ChunkCoordinate(1, 0, "map"));
        // _slotManager->constructChunk(ChunkCoordinate(1, 0, "map"));
        // _slotManager->activateChunk(ChunkCoordinate(1, 0, "map"));
        // _slotManager->activateChunk(ChunkCoordinate(0, 0, "map"));
        // _slotManager->deActivateChunk(ChunkCoordinate(0, 0, "map"));
        // _slotManager->activateChunk(ChunkCoordinate(0, 0, "map"));
        // _slotManager->destroyChunk(ChunkCoordinate(0, 0, "map"));

        //_slotManager->deActivateChunk(ChunkCoordinate(0, 0, "map"));
        //_slotManager->constructChunk(ChunkCoordinate(0, 0, "map"));
    }

    World::~World(){

    }

    SlotPosition pos = SlotPosition();
    void World::update(Ogre::Camera* camera){
        _slotManager->update();

        pos = pos + Ogre::Vector3(1, 0, 0);
        mChunkRadiusLoader->updatePlayer(pos);

        Ogre::Vector3 thing = (pos + Ogre::Vector3(0, 0, 100)).toOgre();
        camera->setPosition(thing);
    }
}
