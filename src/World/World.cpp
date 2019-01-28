#include "World.h"

#include "Slot/SlotPosition.h"
#include "Slot/SlotManager.h"
#include "Slot/ChunkCoordinate.h"
#include "Slot/ChunkRadiusLoader.h"

#include "Input/Input.h"

#include "OgreCamera.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"

namespace AV {
    World::World(){
        _slotManager = std::make_shared<SlotManager>();
        mChunkRadiusLoader = std::make_shared<ChunkRadiusLoader>(_slotManager);
        _slotManager->initialise();

        _slotManager->setCurrentMap("overworld");

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

        EventDispatcher::subscribe(EventType::world, AV_BIND(World::testReceiver));
        
        WorldEvent e;
        EventDispatcher::transmitEvent(EventType::world, e);
    }

    World::~World(){

    }

    SlotPosition pos = SlotPosition();
    void World::update(Ogre::Camera* camera){
        _slotManager->update();

        Ogre::Vector3 ammount = Ogre::Vector3::ZERO;
        if(Input::getKey(Input::Key_Up)) ammount += Ogre::Vector3(0, 0, -1);
        if(Input::getKey(Input::Key_Down)) ammount += Ogre::Vector3(0, 0, 1);
        if(Input::getKey(Input::Key_Left)) ammount += Ogre::Vector3(-1, 0, 0);
        if(Input::getKey(Input::Key_Right)) ammount += Ogre::Vector3(1, 0, 0);

        pos = pos + ammount;
        mChunkRadiusLoader->updatePlayer(pos);

        //AV_INFO(pos);

        Ogre::Vector3 thing = (pos + Ogre::Vector3(0, 100, 100)).toOgre();
        camera->setPosition(thing);

    }

    bool World::testReceiver(const Event &event){
        AV_INFO("Wow event call {}", ((WorldEvent&)event).something);

        return true;
    }
}
