#include "WorldSingleton.h"

namespace AV {
    World* WorldSingleton::_world = 0;
    SlotPosition WorldSingleton::_origin = SlotPosition();
    SlotPosition WorldSingleton::_playerPosition = SlotPosition();
    Ogre::String WorldSingleton::mCurrentMap = "";
    int WorldSingleton::playerLoadRadius = 100;
}
