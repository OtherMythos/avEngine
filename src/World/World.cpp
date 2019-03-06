#include "World.h"

#include "Slot/SlotPosition.h"
#include "Slot/SlotManager.h"
#include "Slot/ChunkCoordinate.h"
#include "Slot/ChunkRadiusLoader.h"

#include "Input/Input.h"

#include "OgreCamera.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"
#include "World/WorldSingleton.h"

namespace AV {
    World::World(){
        EventDispatcher::subscribe(EventType::World, AV_BIND(World::worldEventReceiver));

        _slotManager = std::make_shared<SlotManager>();
        mChunkRadiusLoader = std::make_shared<ChunkRadiusLoader>(_slotManager);
        _slotManager->initialise();

        _slotManager->setCurrentMap("map");
        WorldSingleton::setPlayerPosition(SlotPosition());
    }

    World::~World(){

    }

    SlotPosition pos = SlotPosition();
    bool switchHappened = false;
    int originCount = 30;
    void World::update(Ogre::Camera* camera){
        _slotManager->update();

        Ogre::Vector3 ammount = Ogre::Vector3::ZERO;
        if(Input::getKey(Input::Key_Up)) ammount += Ogre::Vector3(0, 0, -3);
        if(Input::getKey(Input::Key_Down)) ammount += Ogre::Vector3(0, 0, 3);
        if(Input::getKey(Input::Key_Left)) ammount += Ogre::Vector3(-3, 0, 0);
        if(Input::getKey(Input::Key_Right)) ammount += Ogre::Vector3(3, 0, 0);

        if(Input::getKey(Input::Key_Accept) && !switchHappened){
            _slotManager->setCurrentMap("overworld");
            switchHappened = !switchHappened;
        }
        if(Input::getKey(Input::Key_Decline) && originCount <= 0){
            _slotManager->setOrigin(pos);
            originCount = 30;
        }
        if(originCount > 0) originCount--;

        pos = pos + ammount;
        if(ammount != Ogre::Vector3::ZERO) WorldSingleton::setPlayerPosition(pos);

        Ogre::Vector3 thing = pos.toOgre() + Ogre::Vector3(0, 200, 200);
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
