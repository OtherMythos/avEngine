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
        
        void unreferenceScript(int scriptId);
        
    private:
        std::vector<EntityCallbackScript*> mCallbackScripts;
        std::map<Ogre::String, int> mScripts;
    };
}
