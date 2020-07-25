#include "ScriptManager.h"

#include "Script/CallbackScript.h"
#include "Scripting/ScriptVM.h"

#include "System/Util/PathUtils.h"

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
            return mCallbackScripts[foundIdx].lock();
        }

        //The script was not found, meaning it needs to created.

        std::string outString;
        formatResToPath(scriptPath, outString);
        //Check the existance of the file here. Normally the callback script itself would check this for us, but it's more efficient to do it here and save creating a script instance if it doesn't exist.
        if(!fileExists(outString)) return CallbackScriptPtr();

        CallbackScript *s = new CallbackScript();
        ScriptVM::initialiseCallbackScript(s);
        s->prepareRaw(outString.c_str());
        s->mCreatorClass = this;

        CallbackScriptPtr retPtr(s, ScriptManager::_destroyCallbackScript);

        WeakScriptPtr weakPtr(retPtr);

        ScriptId newId = _createLoadedSlot(hashedScript, scriptPath, weakPtr);
        s->mScriptId = newId;

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

    void ScriptManager::_removeScript(ScriptId id){
        assert(mCallbackScripts.size() > id);
        //There's no need to remove the weak reference as it should be marked as expired when the shared pointer goes.

        auto it = mScriptPaths.begin();
        while(it != mScriptPaths.end()){
            if( (*it).second.second == id ){
                mScriptPaths.erase(it);
                return;
            }
            it++;
        }

        //Something should be removed, so this part of the code should never be reached.
        assert(false);
    }

    //Called on shared pointer destruction.
    void ScriptManager::_destroyCallbackScript(CallbackScript* script){
        script->mCreatorClass->_removeScript(script->mScriptId);
        delete script;
    }
}
