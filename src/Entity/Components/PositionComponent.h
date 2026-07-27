#pragma once

#include "World/Slot/SlotPosition.h"

namespace AV{
    struct PositionComponent{
        PositionComponent(SlotPosition pos, bool t) : pos(pos), tracked(t) { }
        PositionComponent(int chunkX, int chunkY, Ogre::Vector3 position) : pos(SlotPosition(chunkX, chunkY, position)) { };

        SlotPosition pos;
        /**
        Whether or not this entity is tracked.
        This is put here for now as a trial.
        All entities need it, and the position component is one all entities have.
        If it does work well having it here the position component will be re-named at a later date.
        */
        bool tracked = false;
    };
}
