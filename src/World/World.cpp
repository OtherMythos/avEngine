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

#include "Entity/EntityManager.h"

namespace AV {
    World::World(){
        EventDispatcher::subscribe(EventType::World, AV_BIND(World::worldEventReceiver));

        mSlotManager = std::make_shared<SlotManager>();
        mChunkRadiusLoader = std::make_shared<ChunkRadiusLoader>(mSlotManager);

        mEntityManager = std::make_shared<EntityManager>();
        mEntityManager->initialise();

        WorldSingleton::setPlayerPosition(SlotPosition());
    }

    World::~World(){

    }

    SlotPosition pos = SlotPosition();
    void World::update(Ogre::Camera* camera){
        mSlotManager->update();

        Ogre::Vector3 ammount = Ogre::Vector3::ZERO;
        if(Input::getKey(Input::Key_Up)) ammount += Ogre::Vector3(0, 0, -3);
        if(Input::getKey(Input::Key_Down)) ammount += Ogre::Vector3(0, 0, 3);
        if(Input::getKey(Input::Key_Left)) ammount += Ogre::Vector3(-3, 0, 0);
        if(Input::getKey(Input::Key_Right)) ammount += Ogre::Vector3(3, 0, 0);

        pos = pos + ammount;
        if(ammount != Ogre::Vector3::ZERO) WorldSingleton::setPlayerPosition(pos);

        camera->setPosition(pos.toOgre() + Ogre::Vector3(0, 200, 200));
    }

    bool World::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        if(event.eventCategory() == WorldEventCategory::MapChange){
            AV_INFO("Map change");
        }

        return true;
    }
}
