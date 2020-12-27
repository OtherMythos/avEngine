#include "EntityCallbackScript.h"

#include "Scripting/Script/CallbackScript.h"
#include "Scripting/ScriptVM.h"
#include "Scripting/ScriptManager.h"
#include "System/BaseSingleton.h"

#include "System/SystemSetup/SystemSettings.h"

#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"
#include "OgreString.h"

#include "World/Entity/eId.h"

namespace AV {
    EntityCallbackScript::EntityCallbackScript()
        : hasUpdateFunction(false) {

    }

    EntityCallbackScript::~EntityCallbackScript(){

    }

    bool EntityCallbackScript::initialise(const Ogre::String &scriptPath){
        //TODO error checking here.
        mScript = BaseSingleton::getScriptManager()->loadScript(scriptPath);
        if(!mScript) return false;

        _scanScriptForEntries();
        return true;
    }

    Ogre::String EntityCallbackScript::getScriptPath(){
        return mScript->getFilePath();
    }

    void EntityCallbackScript::_scanScriptForEntries(){
        static const std::pair<Ogre::String, EntityEventType> callbackMap[] = {
            {"moved", EntityEventType::MOVED},
            {"destroyed", EntityEventType::DESTROYED},
            {"update", EntityEventType::UPDATE},
        };

        static const std::pair<Ogre::String, EntityEventType> componentCallbackMap[] = {
            {"component0Set", EntityEventType::COMPONENT_0},
            {"component1Set", EntityEventType::COMPONENT_1},
            {"component2Set", EntityEventType::COMPONENT_2},
            {"component3Set", EntityEventType::COMPONENT_3},
            {"component4Set", EntityEventType::COMPONENT_4},
            {"component5Set", EntityEventType::COMPONENT_5},
            {"component6Set", EntityEventType::COMPONENT_6},
            {"component7Set", EntityEventType::COMPONENT_7},
            {"component8Set", EntityEventType::COMPONENT_8},
            {"component9Set", EntityEventType::COMPONENT_9},
            {"component10Set", EntityEventType::COMPONENT_10},
            {"component11Set", EntityEventType::COMPONENT_11},
            {"component12Set", EntityEventType::COMPONENT_12},
            {"component13Set", EntityEventType::COMPONENT_13},
            {"component14Set", EntityEventType::COMPONENT_14},
            {"component15Set", EntityEventType::COMPONENT_15}
        };


        for(const std::pair<Ogre::String, EntityEventType>& e : callbackMap){
            _internalScanEntry(e);
        }
        //When checking the component functions, only check as many as have been registered.
        uint8 numComponents = SystemSettings::getUserComponentSettings().numRegisteredComponents;
        for(uint8 i = 0; i < numComponents; i++){
            _internalScanEntry(componentCallbackMap[i]);
        }
        //Check for specific functions which are called as part of a routine.
        hasUpdateFunction = mCallbacks.find(EntityEventType::UPDATE)->second != -1;
    }

    void EntityCallbackScript::_internalScanEntry(const std::pair<Ogre::String, EntityEventType>& e){
        int id = mScript->getCallbackId(e.first);
        uint8 numParams = 0;
        if(id >= 0){
            numParams = mScript->getParamsForCallback(id);
        }
        //The entity callback expects a closure with a single parameter, so with the hidden one that makes 2.
        //If the user provided the incorrect number then invalidate the callback.
        if(numParams != 2) id = -1;

        mCallbacks[e.second] = id;
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
        if(type == EntityEventType::UPDATE) assert(callbackId >= 0);
        if(callbackId < 0) return;

        callbackVariable = entity;

        mScript->call(callbackId, populateEntityEvent);

        //These objects need to be released manually.
        //sq_release(mScript->mVm, &callbackVariable);
    }
}
