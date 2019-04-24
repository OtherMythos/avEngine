#pragma once

#include "EntityCallbackEvents.h"

#include "OgreString.h"
#include <map>
#include "World/Entity/eId.h"

namespace AV {
    class CallbackScript;
    
    class EntityCallbackScript{
    public:
        EntityCallbackScript();
        ~EntityCallbackScript();
        
        void initialise(const Ogre::String &scriptPath);
        void runEntityEvent(eId entity, EntityEventType type);
        
    private:
        CallbackScript *mScript;
        
        void _scanScriptForEntries();
        
        std::map<EntityEventType, int> mCallbacks;
    };
}
