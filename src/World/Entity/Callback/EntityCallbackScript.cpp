#include "EntityCallbackScript.h"

#include "Scripting/Script/CallbackScript.h"
#include "Scripting/ScriptVM.h"
#include "Scripting/ScriptManager.h"
#include "System/BaseSingleton.h"

#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"
#include "OgreString.h"

#include "World/Entity/eId.h"

namespace AV {
    EntityCallbackScript::EntityCallbackScript(){

    }

    EntityCallbackScript::~EntityCallbackScript(){

    }

    void EntityCallbackScript::initialise(const Ogre::String &scriptPath){
        //TODO error checking here.
        mScript = BaseSingleton::getScriptManager()->loadScript(scriptPath);

        _scanScriptForEntries();
    }

    Ogre::String EntityCallbackScript::getScriptPath(){
        return mScript->getFilePath();
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

    //static SQObject callbackVariable;
    static eId callbackVariable;
    SQInteger populateEntityEvent(HSQUIRRELVM vm){
        //sq_pushobject(vm, callbackVariable);
        EntityClass::_entityClassFromEID(vm, callbackVariable);

        //Has to be 2 because we need to incldue the invisible 'this' parameter.
        //I could have added it later on in the call method, but this is more transparent.
        return 2;
    }

    void EntityCallbackScript::runEntityEvent(eId entity, EntityEventType type){
        int callbackId = mCallbacks[type];
        if(callbackId < 0) return;

        callbackVariable = entity;
        //callbackVariable = EntityClass::_objFromEID(mScript->mVm, entity);

        mScript->call(callbackId, populateEntityEvent);

        //These objects need to be released manually.
        //sq_release(mScript->mVm, &callbackVariable);
    }
}
