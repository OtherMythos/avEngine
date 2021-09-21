#include "ScriptingStateManager.h"

#include "ScriptVM.h"
#include "Logger/Log.h"
#include "System/SystemSetup/SystemSettings.h"
#include "Script/CallbackScript.h"
#include "ScriptManager.h"
#include "System/BaseSingleton.h"

#include "System/EngineFlags.h"

namespace AV{
    const std::string ScriptingStateManager::engineStateName = "EngineState";

    ScriptingStateManager::ScriptingStateManager(){

    }

    ScriptingStateManager::~ScriptingStateManager(){

    }

    void ScriptingStateManager::shutdown(){
        //Shutdown all states
        //This is done manually, as then I'm able to call mStates.clear rather than removing them one at a time.
        for(StateEntry& state : mStates){
            _callShutdown(state);
        }
        _callShutdown(mBaseStateEntry.e);
        mBaseStateEntry.e.s.reset();

        mStates.clear();
        AV_INFO("Shut down ScriptingStateManager.");
    }

    void ScriptingStateManager::initialise(){
        _startBaseState();
    }

    bool ScriptingStateManager::_startBaseState(){
        CallbackScriptPtr s = BaseSingleton::getScriptManager()->loadScript(SystemSettings::getSquirrelEntryScriptPath());
        if(!s){
            return false;
        }
        int start = s->getCallbackId("start");
        int update = s->getCallbackId("update");
        int end = s->getCallbackId("end");
        int safeScene = s->getCallbackId("sceneSafeUpdate");

        mBaseStateEntry = {{s, engineStateName, stateEntryStatus::STATE_STARTING, start, update, end}, safeScene};

        return true;
    }

    bool ScriptingStateManager::startState(const std::string& stateName, const std::string& scriptPath){
        //The user can't restart the engine state during engine run because the state will always be there. It's impossible to remove it.
        //As such I don't have to check if the user is trying to start it again, because the normal duplicate state request checks will take care of that.

        bool taken = false;
        for(const StateEntry& state : mStates){
            if(state.stateName == stateName){
                taken = true;
                break;
            }
        }

        if(taken){
            AV_ERROR("The state name {} is already taken.", stateName);
            return false;
        }

        CallbackScriptPtr s = BaseSingleton::getScriptManager()->loadScript(scriptPath);
        if(!s){
            return false;
        }
        int start = s->getCallbackId("start");
        int update = s->getCallbackId("update");
        int end = s->getCallbackId("end");
        mStates.push_back({s, stateName, stateEntryStatus::STATE_STARTING, start, update, end});

        return true;
    }

    bool ScriptingStateManager::stopState(const std::string& stateName){
        if(stateName == engineStateName)
            return false;

        for(StateEntry& state : mStates){
            if(state.stateName == stateName){
                state.stateStatus = stateEntryStatus::STATE_ENDING;
                return true;
            }
        }

        //No state by that name was found.
        return false;
    }

    bool ScriptingStateManager::_updateStateEntry(StateEntry& state){
        bool entryRemoved = false;

        switch(state.stateStatus){
            case stateEntryStatus::STATE_STARTING:
                state.s->call(state.startId);
                state.stateStatus = stateEntryStatus::STATE_RUNNING;
                break;
            case stateEntryStatus::STATE_RUNNING:
                state.s->call(state.updateId);
                break;
            case stateEntryStatus::STATE_ENDING:
                _callShutdown(state);
                entryRemoved = true;
                break;
        }

        return entryRemoved;
    }

    void ScriptingStateManager::updateBaseState(){
        //Call the function to notify that ray queries are in progress.
        if(mBaseStateEntry.e.stateStatus == stateEntryStatus::STATE_RUNNING){
            if(!mBaseStateEntry.e.s.get()) return;
            EngineFlags::_setSceneClear(true);
            mBaseStateEntry.e.s->call(mBaseStateEntry.safeSceneUpdate);
            EngineFlags::_setSceneClear(false);
        }
    }

    void ScriptingStateManager::update(){
        bool entryRemoved = false;

        //OPTIMISATION these checks could be avoided if I put the engine into a 'bad' state when the startup script is broken.
        //From there on I could just assert the script is ok and carry on as normal.
        if(mBaseStateEntry.e.s.get()){
            _updateStateEntry(mBaseStateEntry.e);
        }
        for(StateEntry& state : mStates){
            entryRemoved |= _updateStateEntry(state);
        }

        //No entry needed to be removed.
        if(!entryRemoved) return;

        //Entries need to be removed in the update, but I don't want to do it while the iteration happens.
        //If one was removed I can check here and remove it like that.
        auto it = mStates.begin();
        while(it != mStates.end()){
            if((*it).stateStatus == stateEntryStatus::STATE_ENDING){
                //If the state is ending get rid of it.
                //This erase will destroy the reference, potentially destroying the script.
                it = mStates.erase(it);
            }else it++;
        }
    }

    void ScriptingStateManager::_callShutdown(StateEntry& state){
        state.s->call(state.endId);
    }
}
