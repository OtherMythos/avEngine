#pragma once

#include "OgreString.h"

#include "World/Entity/Callback/EntityCallbackEvents.h"
#include "World/Entity/eId.h"
#include <map>
#include <vector>

namespace AV {
    class EntityCallbackScript;
    
    class EntityCallbackManager{
    public:
        EntityCallbackManager();
        ~EntityCallbackManager();
        
        void initialise();
        
        int loadScript(const Ogre::String &scriptPath);
        
        /**
         Get the index of an already loaded script.
         @return
         -1 if a script with that name is not loaded.
         The index id of the script if it has been loaded.
         */
        int _getScriptIndex(const Ogre::String& filePath);
        
        void notifyEvent(eId entity, EntityEventType type, int scriptId);
        
        /**
         Decrease the reference count of a script.
         */
        void unreferenceScript(int scriptId);
        
        int getActiveScripts() { return mActiveScripts; }
        
    private:
        typedef std::pair<int, EntityCallbackScript*> callbackScriptEntry;
        
        std::vector<callbackScriptEntry> mCallbackScripts;
        std::map<Ogre::String, int> mScripts;
        
        int mActiveScripts = 0;
    };
}
