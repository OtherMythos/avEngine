#include "ScriptPluginManager.h"

#include "ScriptVM.h"
#include "Logger/Log.h"
#include "System/SystemSetup/SystemSettings.h"
#include "Script/CallbackScript.h"
#include "ScriptManager.h"
#include "System/BaseSingleton.h"

#include "System/EngineFlags.h"

namespace AV{
    float ScriptPluginManager::sCurrentDeltaTime = 1.0f / 60.0f;

    SQInteger ScriptPluginManager::_pushDeltaTime(HSQUIRRELVM vm){
        sq_pushfloat(vm, sCurrentDeltaTime);
        return 2; //this + dt
    }

    ScriptPluginManager::ScriptPluginManager(){

    }

    ScriptPluginManager::~ScriptPluginManager(){

    }

    void ScriptPluginManager::initialise(){
        for(const SystemSettings::PluginEntry& e : SystemSettings::getPluginEntries()){
            //A plugin is allowed to be native only, in which case there's nothing to run here.
            if(e.entryFile.empty()) continue;

            CallbackScriptPtr s = BaseSingleton::getScriptManager()->loadScript(e.entryFile);
            if(!s){
                AV_ERROR("Error loading the entry file of plugin '{}' at {}", e.name, e.entryFile);
                continue;
            }

            int start = s->getCallbackId("start");
            int update = s->getCallbackId("update");
            int end = s->getCallbackId("end");
            int sceneSafeUpdate = s->getCallbackId("sceneSafeUpdate");

            uint8 updateParams = 1;
            if(update >= 0) updateParams = s->getParamsForCallback(update);

            mPlugins.push_back({s, e.name, start, update, end, sceneSafeUpdate, updateParams});

            AV_INFO("Started script plugin '{}'", e.name);
        }

        //Started as a separate pass so that a plugin's start function can rely on every other
        //plugin having been loaded, regardless of the order they were declared in.
        for(PluginScriptEntry& p : mPlugins){
            p.s->call(p.startId);
        }
    }

    void ScriptPluginManager::shutdown(){
        for(PluginScriptEntry& p : mPlugins){
            p.s->call(p.endId);
        }

        mPlugins.clear();
        AV_INFO("Shut down ScriptPluginManager.");
    }

    void ScriptPluginManager::update(){
        for(PluginScriptEntry& p : mPlugins){
            if(p.updateParamCount > 1){
                sCurrentDeltaTime = mFixedDeltaTime;
                p.s->call(p.updateId, _pushDeltaTime);
            }else{
                p.s->call(p.updateId);
            }
        }
    }

    void ScriptPluginManager::updateSceneSafe(){
        for(PluginScriptEntry& p : mPlugins){
            //Unlike the other callbacks this one is worth checking up front, as the flag setting
            //below is not free and most plugins won't declare it.
            if(p.sceneSafeUpdateId < 0) continue;

            EngineFlags::_setSceneClear(true);
            p.s->call(p.sceneSafeUpdateId);
            EngineFlags::_setSceneClear(false);
        }
    }
}
