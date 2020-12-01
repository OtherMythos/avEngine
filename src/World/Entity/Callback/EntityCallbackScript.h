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

        Ogre::String getScriptPath();

    private:
        std::shared_ptr<CallbackScript> mScript;

        void _scanScriptForEntries();

        void _internalScanEntry(const std::pair<Ogre::String, EntityEventType>& e);

        std::map<EntityEventType, int> mCallbacks;
    };
}
