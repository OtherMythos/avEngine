#include "ScriptingStateManager.h"

#include "ScriptManager.h"
#include "Logger/Log.h"
#include "System/SystemSetup/SystemSettings.h"

namespace AV{
    const std::string ScriptingStateManager::engineStateName = "EngineState";

    ScriptingStateManager::ScriptingStateManager(){
        initialise();
    }

    ScriptingStateManager::~ScriptingStateManager(){

    }

    void ScriptingStateManager::shutdown(){
        //Shutdown all states
        for(stateEntry& state : mStates){
            _callShutdown(state);
        }

        mStates.clear();
        AV_INFO("Shut down ScriptingStateManager.");
    }

    void ScriptingStateManager::initialise(){
        startState(engineStateName, SystemSettings::getSquirrelEntryScriptPath());
    }

    bool ScriptingStateManager::startState(const std::string stateName, const std::string scriptPath){
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

        Script s;
        ScriptManager::initialiseScript(&s);
        s.compileFile(scriptPath.c_str());
        mStates.push_back({s, stateName, stateEntryStatus::STATE_STARTING});

        return true;
    }

    bool ScriptingStateManager::stopState(const std::string stateName){
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

                //As of right now this is a work around.
                //The functions (closures) have to exist in the root table to be called individually.
                //In order to do that the script has to be run first (because that inserts the correct functions into the root table.)
                //So this is more of a work around than anything.
                //At some point in the near future the Script Manager is going to get a complete re-haul.
                //Much of this stuff will change, and I should be able to avoid this then.
                //TODO the above
                state.s.run();
                state.s.runFunction("start");
                state.stateStatus = stateEntryStatus::STATE_RUNNING;
            }else if(state.stateStatus == stateEntryStatus::STATE_RUNNING){
                //ScriptManager::callFunction(state.scriptFile, "update");
                state.s.runFunction("update");
                state.s.run();
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
                mStates.erase(it);
            }else it++;
        }
    }

    void ScriptingStateManager::_callShutdown(stateEntry& state){
        //ScriptManager::callFunction(state.scriptFile, "end");
        state.s.runFunction("end");
    }
}
