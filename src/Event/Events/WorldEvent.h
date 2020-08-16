#pragma once

#include "Event.h"
#include "OgreString.h"
#include "World/Slot/SlotPosition.h"

namespace AV{

    class World;

    #define AV_WORLD_EVENT_CATEGORY(c) virtual WorldEventCategory eventCategory() const { return c; };

    enum class WorldEventCategory{
        Null,
        MapChange,
        OriginChange,
        PlayerRadiusChange,
        PlayerPositionChange,
        Created,
        Destroyed,
        BecameReady,
        BecameUnready
    };

    class WorldEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_WORLD_EVENT_CATEGORY(WorldEventCategory::Null)
    };

    class WorldEventCreated : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_WORLD_EVENT_CATEGORY(WorldEventCategory::Created)

        World* createdWorld = 0;
        bool createdFromSave = false;
    };

    class WorldEventDestroyed : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_WORLD_EVENT_CATEGORY(WorldEventCategory::Destroyed)
    };

    class WorldEventBecameReady : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_WORLD_EVENT_CATEGORY(WorldEventCategory::BecameReady)
    };

    class WorldEventBecameUnReady : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_WORLD_EVENT_CATEGORY(WorldEventCategory::BecameUnready)
    };

    class WorldEventMapChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_WORLD_EVENT_CATEGORY(WorldEventCategory::MapChange)

        Ogre::String oldMapName;
        Ogre::String newMapName;
    };

    class WorldEventOriginChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_WORLD_EVENT_CATEGORY(WorldEventCategory::OriginChange)

        SlotPosition oldPos;
        SlotPosition newPos;
        Ogre::Vector3 worldOffset;
    };

    class WorldEventPlayerRadiusChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_WORLD_EVENT_CATEGORY(WorldEventCategory::PlayerRadiusChange)

        int oldRadius;
        int newRadius;
    };

    class WorldEventPlayerPositionChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_WORLD_EVENT_CATEGORY(WorldEventCategory::PlayerPositionChange)
    };
}
