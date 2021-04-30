#pragma once

#include "OgreString.h"

#include "World/Entity/Callback/EntityCallbackEvents.h"
#include "World/Entity/eId.h"
#include <map>
#include <vector>

namespace AV {
    class EntityCallbackScript;

    /**
    Manage callbacks for entities.
    This includes managing script lifetime.
    */
    class EntityCallbackManager{
    public:
        EntityCallbackManager();
        ~EntityCallbackManager();

        void initialise();

        /**
        Load a script with res path.
        If the script is already loaded the reference count will be increased.
        Be sure to call unreferenceScript when the script is no longer needed.
        @returns The id of the loaded script.
        */
        int loadScript(const Ogre::String &scriptPath);

        /**
         Get the index of an already loaded script.
         @return
         -1 if a script with that name is not loaded.
         The index id of the script if it has been loaded.
         */
        int _getScriptIndex(const Ogre::String& filePath);

        /**
        Notify an entity that an event of type has occured.
        @param entity The target entity
        @param type The event type which has occured.
        @param scriptId The script which should be called to dispatch the event.
        */
        void notifyEvent(eId entity, EntityEventType type, int scriptId);

        Ogre::String getScriptPath(int scriptId);

        /**
         Decrease the reference count of a script.
         */
        void unreferenceScript(int scriptId);

        int getActiveScripts() { return mActiveScripts; }

        void getMetadataOfScript(int scriptId, bool& hasUpdateFunction) const;

    private:
        //the int represents the reference count
        //If 0 that indicates an empty slot in the vector.
        typedef std::pair<int, EntityCallbackScript*> callbackScriptEntry;

        std::vector<callbackScriptEntry> mCallbackScripts;
        std::map<Ogre::String, int> mScripts;

        int _createLoadedSlot(const Ogre::String &scriptPath, EntityCallbackScript *script);
        int _getLoadedScriptHandle(const Ogre::String &scriptPath);

        int mActiveScripts = 0;

        int _getAvailableIndex();
    };
}
