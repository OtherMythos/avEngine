#pragma once

#include "Event.h"
#include "OgreString.h"
#include "World/Slot/SlotPosition.h"

namespace AV{
    class World;

    class WorldEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_ID(EventId::Null)
    };

    class WorldEventCreated : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_ID(EventId::WorldCreated)

        World* createdWorld = 0;
        bool createdFromSave = false;
    };

    class WorldEventDestroyed : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_ID(EventId::WorldDestroyed)
    };

    class WorldEventBecameReady : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_ID(EventId::WorldBecameReady)
    };

    class WorldEventBecameUnReady : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_ID(EventId::WorldBecameUnready)
    };

    class WorldEventMapChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_ID(EventId::WorldMapChange)

        Ogre::String oldMapName;
        Ogre::String newMapName;
    };

    class WorldEventOriginChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_ID(EventId::WorldOriginChange)

        SlotPosition oldPos;
        SlotPosition newPos;
        Ogre::Vector3 worldOffset;
    };

    class WorldEventPlayerRadiusChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_ID(EventId::WorldPlayerRadiusChange)

        int oldRadius;
        int newRadius;
    };

    class WorldEventPlayerPositionChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_ID(EventId::WorldPlayerPositionChange)
    };
}
