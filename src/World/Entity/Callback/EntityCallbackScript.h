#pragma once

#include "EntityCallbackEvents.h"

#include "OgreString.h"
#include <map>
#include "World/Entity/eId.h"

namespace AV {
    class CallbackScript;

    /**
    Encapsulates the functions specific for entities in a callback script.
    Under the hood this class stores a regular callback script shared object but facilitates what's required for entities.
    This includes initially scanning the script to determine the useable functions.
    */
    class EntityCallbackScript{
    public:
        EntityCallbackScript();
        ~EntityCallbackScript();

        bool initialise(const Ogre::String &scriptPath);
        /**
        Dispatch an entity event.
        Calls the appropriate callback function if it was provided.
        */
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
