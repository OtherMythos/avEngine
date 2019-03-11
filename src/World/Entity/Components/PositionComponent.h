#pragma once

#include "World/Slot/SlotPosition.h"

namespace AV{
    struct PositionComponent{
        PositionComponent(SlotPosition pos) : pos(pos) { }
        PositionComponent(int chunkX, int chunkY, Ogre::Vector3 position) : pos(SlotPosition(chunkX, chunkY, position)) { };

        SlotPosition pos;
    };
}
