#include "PhysicsCollisionDataManager.h"

#include "System/BaseSingleton.h"
#include "Scripting/ScriptManager.h"
#include "Scripting/Script/CallbackScript.h"
#include "Worlds/CollisionWorldUtils.h"

#include "btBulletDynamicsCommon.h"

namespace AV{

    ScriptDataPacker<PhysicsCollisionDataManager::CollisionSenderScriptEntry> PhysicsCollisionDataManager::mSenderScriptObjects;

    void* PhysicsCollisionDataManager::createCollisionSenderScriptFromData(const std::string& scriptPath, const std::string& funcName, int id){
        std::shared_ptr<CallbackScript> script = BaseSingleton::getScriptManager()->loadScript(scriptPath);
        int callbackId = script->getCallbackId(funcName);
        if(callbackId < 0); //TODO Do something as error handling.

        void* retVal = mSenderScriptObjects.storeEntry({script, callbackId, id});

        return retVal;
    }

    void PhysicsCollisionDataManager::processCollision(const btCollisionObject* sender, const btCollisionObject* receiver){
        CollisionWorldUtils::PackedIntContents contents;
        CollisionWorldUtils::readPackedInt(sender->getUserIndex(), &contents);

        //Assume it's a sender script. When I have sound effects this would be something else.
        const PhysicsCollisionDataManager::CollisionSenderScriptEntry& data = mSenderScriptObjects.getEntry(sender->getUserPointer());
        //In future I'm going to pass some values in here. Right now it calls an empty function.
        data.scriptPtr->call(data.closureId);
    }
}
