#include "PhysicsCollisionDataManager.h"

#include "System/BaseSingleton.h"
#include "Scripting/ScriptManager.h"
#include "Scripting/Script/CallbackScript.h"
#include "Worlds/CollisionWorldUtils.h"

#include "btBulletDynamicsCommon.h"

namespace AV{

    ScriptDataPacker<PhysicsCollisionDataManager::CollisionSenderScriptEntry> PhysicsCollisionDataManager::mSenderScriptObjects;

    void PhysicsCollisionDataManager::shutdown(){
        mSenderScriptObjects.clear();
    }

    void* PhysicsCollisionDataManager::createCollisionSenderScriptFromData(const std::string& scriptPath, const std::string& funcName, int id){
        std::shared_ptr<CallbackScript> script = BaseSingleton::getScriptManager()->loadScript(scriptPath);
        int callbackId = script->getCallbackId(funcName);
        if(callbackId < 0); //TODO Do something as error handling.

        void* retVal = mSenderScriptObjects.storeEntry({script, callbackId, id});

        return retVal;
    }


    //Populate functions
    static SQInteger targetCollisionUserId = 0;
    static CollisionObjectEventMask::CollisionObjectEventMask targetEventMask;


    SQInteger populateSenderId(HSQUIRRELVM vm){
        sq_pushinteger(vm, targetCollisionUserId);

        return 2;
    }

    SQInteger populateSenderIdEventMask(HSQUIRRELVM vm){
        sq_pushinteger(vm, targetCollisionUserId);
        sq_pushinteger(vm, (SQInteger)targetEventMask);

        return 3;
    }

    void PhysicsCollisionDataManager::processCollision(const btCollisionObject* sender, const btCollisionObject* receiver, CollisionObjectEventMask::CollisionObjectEventMask eventMask){
        CollisionWorldUtils::PackedIntContents contents;
        CollisionWorldUtils::readPackedInt(sender->getUserIndex(), &contents);

        //Assume it's a sender script. When I have sound effects this would be something else.
        const PhysicsCollisionDataManager::CollisionSenderScriptEntry& data = mSenderScriptObjects.getEntry(sender->getUserPointer());

        //Invalid closure.
        if(data.closureId < 0) return;

        uint8 numParams = data.scriptPtr->getParamsForCallback(data.closureId);
        PopulateFunction populateFunction = 0;

        switch(numParams){
            case 1:
                //Just an empty function call.
                break;
            case 2:
                populateFunction = &populateSenderId;
                targetCollisionUserId = data.userIndex;
                break;
            case 3:
                populateFunction = &populateSenderIdEventMask;
                targetCollisionUserId = data.userIndex;
                targetEventMask = eventMask;
                break;
            default:
                //Nothing to call as none of the functions matched up.
                return;
        };

        data.scriptPtr->call(data.closureId, populateFunction);
    }
}
