#include "EntityCallbackManager.h"

#include "EntityCallbackScript.h"

namespace AV {
    EntityCallbackManager::EntityCallbackManager(){
        
    }
    
    EntityCallbackManager::~EntityCallbackManager(){
        
    }
    
    void EntityCallbackManager::initialise(){
        for(EntityCallbackScript *e :  mCallbackScripts){
            delete e;
        }
    }
    
    int EntityCallbackManager::_getScriptIndex(const Ogre::String& filePath){
        auto it = mScripts.find(filePath);
        if(it == mScripts.end()) return -1;
        
        return (*it).second;
    }
    
    void EntityCallbackManager::unreferenceScript(int scriptId){
        
    }
    
    int EntityCallbackManager::loadScript(const Ogre::String &scriptPath){
        int foundScriptIndex = _getScriptIndex(scriptPath);
        //If that script has already been loaded, just return the id of that script.
        if(foundScriptIndex >= 0) return foundScriptIndex;
        
        EntityCallbackScript *s = new EntityCallbackScript();
        //TODO rather than pushing to the back of the array this should search the array to find a 0 value and replace with that.
        mCallbackScripts.push_back(s);
        
        int indexId = mCallbackScripts.size() - 1;
        mScripts[scriptPath] = indexId;
        
        s->initialise(scriptPath);
        
        return indexId;
    }
    
    void EntityCallbackManager::notifyEvent(eId entity, EntityEventType type, int scriptId){
        mCallbackScripts[scriptId]->runEntityEvent(entity, type);
    }
}
