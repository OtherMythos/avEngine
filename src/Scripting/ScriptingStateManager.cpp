#include "ScriptingStateManager.h"

#include "ScriptManager.h"
#include "Logger/Log.h"
#include "System/SystemSetup/SystemSettings.h"
#include "Script/CallbackScript.h"

namespace AV{
    const std::string ScriptingStateManager::engineStateName = "EngineState";

    ScriptingStateManager::ScriptingStateManager(){
        initialise();
    }

    ScriptingStateManager::~ScriptingStateManager(){

    }

    void ScriptingStateManager::shutdown(){
        //Shutdown all states
        //This is done manually, as then I'm able to call mStates.clear rather than removing them one at a time.
        for(stateEntry& state : mStates){
            _callShutdown(state);
            _destroyStateEntry(state);
        }

        mStates.clear();
        AV_INFO("Shut down ScriptingStateManager.");
    }

    void ScriptingStateManager::initialise(){
        startState(engineStateName, SystemSettings::getSquirrelEntryScriptPath());
    }

    bool ScriptingStateManager::startState(const std::string& stateName, const std::string& scriptPath){
        //The user can't restart the engine state during engine run because the state will always be there. It's impossible to remove it.
        //As such I don't have to check if the user is trying to start it again, because the normal duplicate state request checks will take care of that.

        bool taken = false;
        for(const stateEntry& state : mStates){
            if(state.stateName == stateName){
                taken = true;
                break;
            }
        }

        if(taken){
            AV_ERROR("The state name {} is already taken.", stateName);
            return false;
        }

        CallbackScript *s = new CallbackScript();
        ScriptManager::initialiseCallbackScript(s);
        s->prepare(scriptPath.c_str());
        int start = s->getCallbackId("start");
        int update = s->getCallbackId("update");
        int end = s->getCallbackId("end");
        mStates.push_back({s, stateName, stateEntryStatus::STATE_STARTING, start, update, end});

        return true;
    }

    bool ScriptingStateManager::stopState(const std::string& stateName){
        if(stateName == engineStateName)
            return false;

        for(stateEntry& state : mStates){
            if(state.stateName == stateName){
                state.stateStatus = stateEntryStatus::STATE_ENDING;
                return true;
            }
        }

        //No state by that name was found.
        return false;
    }

    void ScriptingStateManager::update(){
        bool entryRemoved = false;

        for(stateEntry& state : mStates){
            if(state.stateStatus == stateEntryStatus::STATE_STARTING){
                state.s->call(state.startId);
                state.stateStatus = stateEntryStatus::STATE_RUNNING;
            }else if(state.stateStatus == stateEntryStatus::STATE_RUNNING){
                state.s->call(state.updateId);
            }else if(state.stateStatus == stateEntryStatus::STATE_ENDING){
                _callShutdown(state);
                entryRemoved = true;
            }
        }

        //No entry needed to be removed.
        if(!entryRemoved) return;

        //Entries need to be removed in the update, but I don't want to do it while the iteration happens.
        //If one was removed I can check here and remove it like that.
        auto it = mStates.begin();
        while(it != mStates.end()){
            if((*it).stateStatus == stateEntryStatus::STATE_ENDING){
                //If the state is ending get rid of it.
                _destroyStateEntry(*it);
                mStates.erase(it);
            }else it++;
        }
    }

    void ScriptingStateManager::_callShutdown(stateEntry& state){
        state.s->call(state.endId);
    }

    void ScriptingStateManager::_destroyStateEntry(stateEntry& state){
        delete state.s;
    }
}
