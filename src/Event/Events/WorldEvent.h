#pragma once

#include "Event.h"
#include "OgreString.h"

namespace AV{

    #define AV_EVENT_CATEGORY(c) virtual WorldEventCategory eventCategory() const { return c; };

    enum class WorldEventCategory{
        Null,
        MapChange,
        OriginChange,
        PlayerRadiusChange,
        PlayerPositionChange
    };

    class WorldEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_CATEGORY(WorldEventCategory::Null)
    };

    class WorldEventMapChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_CATEGORY(WorldEventCategory::MapChange)

        Ogre::String oldMapName;
        Ogre::String newMapName;
    };

    class WorldEventOriginChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_CATEGORY(WorldEventCategory::OriginChange)

        SlotPosition oldPos;
        SlotPosition newPos;
    };

    class WorldEventPlayerRadiusChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_CATEGORY(WorldEventCategory::PlayerRadiusChange)

        int oldRadius;
        int newRadius;
    };

    class WorldEventPlayerPositionChange : public WorldEvent{
    public:
        AV_EVENT_TYPE(EventType::World)
        AV_EVENT_CATEGORY(WorldEventCategory::PlayerPositionChange)


    };
}
