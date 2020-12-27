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

        bool initialise(const Ogre::String &scriptPath);
        void runEntityEvent(eId entity, EntityEventType type);

        Ogre::String getScriptPath();

        bool getHasUpdateFunction() const { return hasUpdateFunction; }

    private:
        std::shared_ptr<CallbackScript> mScript;

        void _scanScriptForEntries();

        void _internalScanEntry(const std::pair<Ogre::String, EntityEventType>& e);

        bool hasUpdateFunction;

        std::map<EntityEventType, int> mCallbacks;
    };
}
