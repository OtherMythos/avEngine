#include "WorldSingleton.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"

namespace AV {
    World* WorldSingleton::_world = 0;
    SlotPosition WorldSingleton::_origin = SlotPosition();
    SlotPosition WorldSingleton::_playerPosition = SlotPosition();
    Ogre::String WorldSingleton::mCurrentMap = "";
    int WorldSingleton::playerLoadRadius = 200;

    void WorldSingleton::setPlayerLoadRadius(int radius){
        WorldEventPlayerRadiusChange event;
        event.oldRadius = playerLoadRadius;
        event.newRadius = radius;

        playerLoadRadius = radius;

        EventDispatcher::transmitEvent(EventType::World, event);
    }

    void WorldSingleton::setPlayerPosition(const SlotPosition& pos){
        WorldEventPlayerPositionChange event;

        _playerPosition = pos;

        EventDispatcher::transmitEvent(EventType::World, event);
    }
}
