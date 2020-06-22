#include "ScriptManager.h"

#include "Script/CallbackScript.h"
#include "Scripting/ScriptVM.h"

namespace AV{
    ScriptManager::ScriptManager(){

    }

    ScriptManager::~ScriptManager(){

    }

    ScriptManager::ScriptId ScriptManager::_findScript(const Ogre::IdString& id){
        auto it = mScriptPaths.find(id);
        if(it == mScriptPaths.end()) return INVALID_SCRIPT_ID;

        return (*it).second.second;
    }

    ScriptManager::ScriptId ScriptManager::_getAvailableIndex(){
        for(int i = 0; i < mCallbackScripts.size(); i++){
            if(mCallbackScripts[i].expired()) return i;
        }

        return INVALID_SCRIPT_ID;
    }

    CallbackScriptPtr ScriptManager::loadScript(const std::string& scriptPath){
        Ogre::IdString hashedScript(scriptPath);
        ScriptId foundIdx = _findScript(hashedScript);
        if(foundIdx != INVALID_SCRIPT_ID){
            //This script has already been created. All we need to do is increase the reference.
            // assert(mCallbackScripts.size() > foundIdx);
            // mCallbackScripts[foundIdx].first++;
            // return foundIdx;
            return mCallbackScripts[foundIdx].lock();
        }

        //The script was not found, meaning it needs to created.
        CallbackScript *s = new CallbackScript();
        ScriptVM::initialiseCallbackScript(s);
        s->prepare(scriptPath.c_str());

        CallbackScriptPtr retPtr(s, ScriptManager::_destroyCallbackScript);

        WeakScriptPtr weakPtr(retPtr);

        _createLoadedSlot(hashedScript, scriptPath, weakPtr);

        return retPtr;
    }

    ScriptManager::ScriptId ScriptManager::_createLoadedSlot(const Ogre::IdString& hashedPath, const std::string &scriptPath, WeakScriptPtr script){
        ScriptId targetIndex = _getAvailableIndex();
        //CallbackScriptEntry entry(1, script); //1 because when a script is first loaded it would only have a single reference.
        if(targetIndex != INVALID_SCRIPT_ID){
            mCallbackScripts[targetIndex] = script;
        }else{
            mCallbackScripts.push_back(script);
            targetIndex = mCallbackScripts.size() - 1;
        }

        mScriptPaths[hashedPath] = {scriptPath, targetIndex};

        return targetIndex;
    }

    // void ScriptManager::unreferenceScript(ScriptId scriptId){
    //     assert(mCallbackScripts.size() > scriptId);
    //     mCallbackScripts[scriptId].first--;
    //     if(mCallbackScripts[scriptId].first <= 0){
    //         //The script no longer holds any references, so it can be removed.
    //         _removeScript(scriptId);
    //     }
    // }

    // void ScriptManager::_removeScript(ScriptId id){
    //     delete mCallbackScripts[id].second;
    //
    //     //Entries in the vector are never actually removed, they're just set to null.
    //     //This is more efficient, and helps to make sure ids are always valid.
    //     //ie, if a script had id 5, and the 4 before it where removed, the index would now be invalid.
    //     //So the 4 before are not removed but set to 0.
    //     //If something then later requests a slot it will try and find one with 0 before pushing back.
    //     mCallbackScripts[id] = CallbackScriptEntry(0, 0);
    //
    //     auto it = mScriptPaths.begin();
    //     while(it != mScriptPaths.end()){
    //         if((*it).second.second == id){
    //             //The entry to be removed has been found.
    //             mScriptPaths.erase(it);
    //             break;
    //         }
    //
    //         it++;
    //     }
    // }

    //Called on shared pointer destruction.
    void ScriptManager::_destroyCallbackScript(CallbackScript* script){
        delete script;
    }
}
