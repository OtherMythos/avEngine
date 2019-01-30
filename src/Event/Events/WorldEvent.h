#pragma once

#include "Event.h"
#include "OgreString.h"

namespace AV{
    
    #define AV_EVENT_CATEGORY(c) virtual WorldEventCategory eventCategory() const { return c; };
    
    enum class WorldEventCategory{
        Null,
        MapChange
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
        
        Ogre::String mapName;
    };
}
