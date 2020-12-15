#include "EntityCallbackManager.h"

#include "EntityCallbackScript.h"
#include "Logger/Log.h"

namespace AV {
    EntityCallbackManager::EntityCallbackManager(){

    }

    EntityCallbackManager::~EntityCallbackManager(){
        AV_INFO("Shutting down the EntityCallbackManager");
        for(const callbackScriptEntry& e : mCallbackScripts){
            delete e.second;
        }
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

    Ogre::String EntityCallbackManager::getScriptPath(int scriptId){
        if(scriptId < 0 || scriptId >= mCallbackScripts.size()) return "";

        return mCallbackScripts[scriptId].second->getScriptPath();
    }

    int EntityCallbackManager::_getAvailableIndex(){
        for(int i = 0; i < mCallbackScripts.size(); i++){
            if(mCallbackScripts[i].first == 0) return i;
        }

        return -1;
    }

    void EntityCallbackManager::unreferenceScript(int scriptId){
        mCallbackScripts[scriptId].first--;
        if(mCallbackScripts[scriptId].first <= 0){
            //The script no longer holds any references, so it can be removed.

            delete mCallbackScripts[scriptId].second;

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
        int scriptHandle = _getLoadedScriptHandle(scriptPath);
        if(scriptHandle >= 0) return scriptHandle;

        EntityCallbackScript *s = new EntityCallbackScript();
        s->initialise(scriptPath);
        return _createLoadedSlot(scriptPath, s);
    }

    int EntityCallbackManager::_getLoadedScriptHandle(const Ogre::String &scriptPath){
        int foundScriptIndex = _getScriptIndex(scriptPath);
        //If that script has already been loaded, just return the id of that script.
        if(foundScriptIndex >= 0) {
            mCallbackScripts[foundScriptIndex].first++;
            return foundScriptIndex;
        }

        return -1;
    }

    int EntityCallbackManager::_createLoadedSlot(const Ogre::String &scriptPath, EntityCallbackScript *script){
        //Here I do a check to see if there's a gap in the vector (script with a 0 value).
        //If there is I can use that rather than creating a new entry.
        int targetIndex = _getAvailableIndex();
        callbackScriptEntry entry(1, script); //1 because when a script is first loaded it would only have a single reference.
        if(targetIndex >= 0){
            mCallbackScripts[targetIndex] = entry;
        }else{
            mCallbackScripts.push_back(entry);
            targetIndex = mCallbackScripts.size() - 1;
        }

        mScripts[scriptPath] = targetIndex;

        mActiveScripts++;

        return targetIndex;
    }

    void EntityCallbackManager::notifyEvent(eId entity, EntityEventType type, int scriptId){
        mCallbackScripts[scriptId].second->runEntityEvent(entity, type);
    }

    void EntityCallbackManager::getMetadataOfScript(int scriptId, bool& hasUpdateFunction) const{
        hasUpdateFunction = mCallbackScripts[scriptId].second->getHasUpdateFunction();
    }
}
