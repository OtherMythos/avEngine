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
        EventDispatcher::subscribe(EventType::World, AV_BIND(World::worldEventReceiver));
        
        _slotManager = std::make_shared<SlotManager>();
        mChunkRadiusLoader = std::make_shared<ChunkRadiusLoader>(_slotManager);
        _slotManager->initialise();

        _slotManager->setCurrentMap("map");
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

        Ogre::Vector3 thing = (pos + Ogre::Vector3(0, 200, 200)).toOgre();
        camera->setPosition(thing);

    }

    bool World::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        if(event.eventCategory() == WorldEventCategory::MapChange){
            AV_INFO("Map change");
        }

        return true;
    }
}
