#include "EntityCallbackManager.h"

#include "EntityCallbackScript.h"

namespace AV {
    EntityCallbackManager::EntityCallbackManager(){
        
    }
    
    EntityCallbackManager::~EntityCallbackManager(){
        
    }
    
    void EntityCallbackManager::initialise(){
        for(callbackScriptEntry e :  mCallbackScripts){
            delete e.second;
        }
    }
    
    int EntityCallbackManager::_getScriptIndex(const Ogre::String& filePath){
        auto it = mScripts.find(filePath);
        if(it == mScripts.end()) return -1;
        
        return (*it).second;
    }
    
    void EntityCallbackManager::unreferenceScript(int scriptId){
        mCallbackScripts[scriptId].first--;
        if(mCallbackScripts[scriptId].first <= 0){
            //The script no longer holds any references, so it can be removed.
            
            //Entries in the vector are never actually removed, they're just set to null.
            //This is more efficient, and helps to make sure ids are always valid.
            //ie, if a script had id 5, and the 4 before it where removed, the index would now be invalid.
            //So the 4 before are not removed but set to 0.
            //If something then later requests a slot it will try and find one with 0 before pushing back.
            mCallbackScripts[scriptId] = callbackScriptEntry(0, 0);
            
            auto it = mScripts.begin();
            while(it != mScripts.end()){
                if((*it).second == scriptId){
                    //The entry to be removed has been found.
                    mScripts.erase(it);
                    break;
                }
                
                it++;
            }
            mActiveScripts--;
        }
    }
    
    int EntityCallbackManager::loadScript(const Ogre::String &scriptPath){
        int foundScriptIndex = _getScriptIndex(scriptPath);
        //If that script has already been loaded, just return the id of that script.
        if(foundScriptIndex >= 0) {
            mCallbackScripts[foundScriptIndex].first++;
            return foundScriptIndex;
        }
        
        EntityCallbackScript *s = new EntityCallbackScript();
        //TODO rather than pushing to the back of the array this should search the array to find a 0 value and replace with that.
        //1 because when a script is first loaded it would only have a single reference.
        mCallbackScripts.push_back(callbackScriptEntry(1, s));
        
        int indexId = mCallbackScripts.size() - 1;
        mScripts[scriptPath] = indexId;
        
        s->initialise(scriptPath);
        mActiveScripts++;
        
        return indexId;
    }
    
    void EntityCallbackManager::notifyEvent(eId entity, EntityEventType type, int scriptId){
        mCallbackScripts[scriptId].second->runEntityEvent(entity, type);
    }
}
