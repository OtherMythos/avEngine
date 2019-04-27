#include "EntityCallbackScript.h"
#include "Scripting/Script/CallbackScript.h"
#include "Scripting/ScriptManager.h"

#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"

#include "World/Entity/eId.h"

namespace AV {
    EntityCallbackScript::EntityCallbackScript(){
        
    }
    
    EntityCallbackScript::~EntityCallbackScript(){
        
    }
    
    void EntityCallbackScript::initialise(const Ogre::String &scriptPath){
        mScript = new CallbackScript();
        ScriptManager::initialiseCallbackScript(mScript);
        
        if(!mScript->prepare(scriptPath)) return;
        
        _scanScriptForEntries();
    }
    
    void EntityCallbackScript::_scanScriptForEntries(){
        static const std::map<Ogre::String, EntityEventType> callbackMap = {
            {"moved", EntityEventType::MOVED},
            {"destroyed", EntityEventType::DESTROYED}
        };
        
        for(const std::pair<Ogre::String, EntityEventType>& e : callbackMap){
            int id = mScript->getCallbackId(e.first);
            
            mCallbacks[e.second] = id;
        }
    }
    
    void EntityCallbackScript::runEntityEvent(eId entity, EntityEventType type){
        int callbackId = mCallbacks[type];
        if(callbackId < 0) return;
        
        //OPTIMISATION
        //There should be a way to scan the closure on startup to see if it takes any parameters.
        //If it doesn't, then this can be avoided.
        //This would be more efficient, as the eid won't necessarily be used each time.
        SQObject obj = EntityClass::_objFromEID(mScript->mVm, entity);
        
        mScript->call(callbackId, &obj);
    }
}
