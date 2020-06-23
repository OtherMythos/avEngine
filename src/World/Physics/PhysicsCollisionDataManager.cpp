#include "PhysicsCollisionDataManager.h"

#include "System/BaseSingleton.h"
#include "Scripting/ScriptManager.h"
#include "Scripting/Script/CallbackScript.h"

namespace AV{

    ScriptDataPacker<PhysicsCollisionDataManager::CollisionSenderScriptEntry> PhysicsCollisionDataManager::mSenderScriptObjects;

    void* PhysicsCollisionDataManager::createCollisionSenderScriptFromData(const std::string& scriptPath, const std::string& funcName, int id){
        std::shared_ptr<CallbackScript> script = BaseSingleton::getScriptManager()->loadScript(scriptPath);
        int callbackId = script->getCallbackId(funcName);
        if(callbackId < 0); //TODO Do something as error handling.

        void* retVal = mSenderScriptObjects.storeEntry({script, callbackId, id});

        return retVal;
    }
}
