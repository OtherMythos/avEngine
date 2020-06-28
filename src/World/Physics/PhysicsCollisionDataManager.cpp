#include "PhysicsCollisionDataManager.h"

#include "System/BaseSingleton.h"
#include "Scripting/ScriptManager.h"
#include "Scripting/Script/CallbackScript.h"
#include "Worlds/CollisionWorldUtils.h"

#include "btBulletDynamicsCommon.h"

namespace AV{

    ScriptDataPacker<PhysicsCollisionDataManager::CollisionSenderScriptEntry> PhysicsCollisionDataManager::mSenderScriptObjects;
    ScriptDataPacker<PhysicsCollisionDataManager::CollisionSenderClosureEntry> PhysicsCollisionDataManager::mSenderClosureObjects;

    void PhysicsCollisionDataManager::shutdown(){
        mSenderScriptObjects.clear();

        //TODO I need some way to iterate the closure list and remove references to whatever's left.
    }

    void* PhysicsCollisionDataManager::createCollisionSenderScriptFromData(const std::string& scriptPath, const std::string& funcName, int id){
        std::shared_ptr<CallbackScript> script = BaseSingleton::getScriptManager()->loadScript(scriptPath);
        int callbackId = script->getCallbackId(funcName);
        if(callbackId < 0); //TODO Do something as error handling.

        void* retVal = mSenderScriptObjects.storeEntry({script, callbackId, {id} });

        return retVal;
    }

    void* PhysicsCollisionDataManager::createCollisionSenderClosureFromData(HSQUIRRELVM vm, SQObject closure, uint8 closureParams, int id){
        //Increase the references of the closure here. This way it won't be deleted by squirrel.
        assert(closure._type == OT_CLOSURE);
        sq_addref(vm, &closure);

        void* retVal = mSenderClosureObjects.storeEntry({closure, closureParams, {id} });

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

    void PhysicsCollisionDataManager::_processCollisionClosure(void* closureEntry, CollisionObjectEventMask::CollisionObjectEventMask eventMask){
        const PhysicsCollisionDataManager::CollisionSenderClosureEntry& data = mSenderClosureObjects.getEntry(closureEntry);

        PopulateFunction populateFunction = 0;
        if(!_determinePopulateFunction(data.numParams, data.userData, eventMask, &populateFunction)) return;

        ScriptVM::callClosure(data.closure, 0, populateFunction);
    }

    void PhysicsCollisionDataManager::_processCollisionScript(void* scriptEntry, CollisionObjectEventMask::CollisionObjectEventMask eventMask){
        const PhysicsCollisionDataManager::CollisionSenderScriptEntry& data = mSenderScriptObjects.getEntry(scriptEntry);

        //Invalid closure.
        if(data.closureId < 0) return;

        uint8 numParams = data.scriptPtr->getParamsForCallback(data.closureId);
        PopulateFunction populateFunction = 0;

        if(!_determinePopulateFunction(numParams, data.userData, eventMask, &populateFunction)) return;

        data.scriptPtr->call(data.closureId, populateFunction);
    }

    void PhysicsCollisionDataManager::processCollision(const btCollisionObject* sender, const btCollisionObject* receiver, CollisionObjectEventMask::CollisionObjectEventMask eventMask){
        CollisionWorldUtils::PackedIntContents contents;
        CollisionWorldUtils::readPackedInt(sender->getUserIndex(), &contents);

        switch(contents.type){
            case CollisionObjectType::SENDER_SCRIPT:
                _processCollisionScript(sender->getUserPointer(), eventMask);
                break;
            case CollisionObjectType::SENDER_CLOSURE:
                _processCollisionClosure(sender->getUserPointer(), eventMask);
                break;
            default:
                assert(false);
                break;
        }
    }

    bool PhysicsCollisionDataManager::_determinePopulateFunction(uint8 numParams, const CollisionSenderUserData& data, CollisionObjectEventMask::CollisionObjectEventMask eventMask, PopulateFunction* outFunc){
        switch(numParams){
            case 1:
                //Just an empty function call.
                break;
            case 2:
                *outFunc = &populateSenderId;
                targetCollisionUserId = data.userIndex;
                break;
            case 3:
                *outFunc = &populateSenderIdEventMask;
                targetCollisionUserId = data.userIndex;
                targetEventMask = eventMask;
                break;
            default:
                //Nothing to call as none of the functions matched up.
                return false;
        };

        return true;
    }
}
