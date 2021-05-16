#pragma once

namespace AV{
    enum PAUSE_TYPES{
        PAUSE_TYPE_PHYSICS = 1u << 0,
        PAUSE_TYPE_PHYSICS_DYNAMICS = 1u << 1,
        PAUSE_TYPE_PHYSICS_COLLISION = 1u << 2,
        PAUSE_TYPE_PHYSICS_COLLISION0 = 1u << 3,
        PAUSE_TYPE_PHYSICS_COLLISION1 = 1u << 4,
        PAUSE_TYPE_PHYSICS_COLLISION2 = 1u << 5,
        PAUSE_TYPE_PHYSICS_COLLISION3 = 1u << 6,
        PAUSE_TYPE_NAV_MESH = 1u << 7,
        PAUSE_TYPE_LIFETIME_COMPONENT = 1u << 8,
        PAUSE_TYPE_ANIMATIONS = 1u << 9,
        PAUSE_TYPE_TIMERS = 1u << 10,
        PAUSE_TYPE_ENTITY_UPDATE = 1u << 11,
    };

    static const char* const PauseTypeStrs[] = {
        "_PAUSE_PHYSICS",
        "_PAUSE_PHYSICS_DYNAMICS",
        "_PAUSE_PHYSICS_COLLISION",
        "_PAUSE_PHYSICS_COLLISION0",
        "_PAUSE_PHYSICS_COLLISION1",
        "_PAUSE_PHYSICS_COLLISION2",
        "_PAUSE_PHYSICS_COLLISION3",
        "_PAUSE_NAV_MESH",
        "_PAUSE_LIFETIME_COMPONENT",
        "_PAUSE_ANIMATIONS",
        "_PAUSE_TIMERS",
        "_PAUSE_ENTITY_UPDATE",
    };

    typedef unsigned int PauseMask;
}
