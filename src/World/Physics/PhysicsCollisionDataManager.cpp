#include "PhysicsCollisionDataManager.h"

#include "System/BaseSingleton.h"
#include "Scripting/ScriptManager.h"
#include "Scripting/Script/CallbackScript.h"
#include "Worlds/CollisionWorldUtils.h"
#include "World/Physics/PhysicsMetaDataManager.h"
#include "Scripting/ScriptNamespace/Classes/Entity/EntityUserData.h"

#include "btBulletDynamicsCommon.h"

namespace AV{

    DataPacker<PhysicsCollisionDataManager::CollisionSenderScriptEntry> PhysicsCollisionDataManager::mSenderScriptObjects;
    DataPacker<PhysicsCollisionDataManager::CollisionSenderClosureEntry> PhysicsCollisionDataManager::mSenderClosureObjects;
    SQObject PhysicsCollisionDataManager::overrideFunction;

    void PhysicsCollisionDataManager::startup(){
        sq_resetobject(&overrideFunction);
    }

    void PhysicsCollisionDataManager::shutdown(){
        mSenderScriptObjects.clear();

        for(const std::pair<PhysicsCollisionDataManager::CollisionSenderClosureEntry, int>& e : mSenderClosureObjects.getInternalData()){
            ScriptVM::dereferenceObject(e.first.closure);
        }
        mSenderClosureObjects.clear();

        SQObject obj;
        sq_resetobject(&obj);
        setCollisionCallbackOverride(obj);
    }

    void* PhysicsCollisionDataManager::createCollisionSenderScriptFromData(std::shared_ptr<CallbackScript> script, const std::string& funcName, int id){
        if(!script) return INVALID_DATA_ID;
        int callbackId = script->getCallbackId(funcName);
        if(callbackId < 0) return INVALID_DATA_ID;

        void* retVal = mSenderScriptObjects.storeEntry({script, callbackId, {id} });

        return retVal;
    }

    void* PhysicsCollisionDataManager::createCollisionSenderScriptFromData(const std::string& scriptPath, const std::string& funcName, int id){
        std::shared_ptr<CallbackScript> script = BaseSingleton::getScriptManager()->loadScript(scriptPath);
        if(script == 0) return INVALID_DATA_ID;

        return createCollisionSenderScriptFromData(script, funcName, id);
    }

    void* PhysicsCollisionDataManager::createCollisionSenderClosureFromData(SQObject closure, uint8 closureParams, int id){
        //Increase the references of the closure here. This way it won't be deleted by squirrel.
        ScriptVM::referenceObject(closure);

        void* retVal = mSenderClosureObjects.storeEntry({closure, closureParams, {id} });

        return retVal;
    }


    //Populate functions
    static SQInteger targetCollisionUserId = 0;
    static CollisionObjectEventMask::CollisionObjectEventMask targetEventMask;
    static SQInteger internalCollisionId = 0;
    static eId targetSenderId = eId::INVALID;
    static eId targetReceiverId = eId::INVALID;


    SQInteger populateFunc1(HSQUIRRELVM vm){
        sq_pushinteger(vm, targetCollisionUserId);

        return 2;
    }

    SQInteger populateFunc2(HSQUIRRELVM vm){
        sq_pushinteger(vm, targetCollisionUserId);
        sq_pushinteger(vm, (SQInteger)targetEventMask);

        return 3;
    }

    SQInteger populateFunc3(HSQUIRRELVM vm){
        sq_pushinteger(vm, targetCollisionUserId);
        sq_pushinteger(vm, (SQInteger)targetEventMask);
        sq_pushinteger(vm, internalCollisionId);

        return 4;
    }

    SQInteger populateFunc4(HSQUIRRELVM vm){
        sq_pushinteger(vm, targetCollisionUserId);
        sq_pushinteger(vm, (SQInteger)targetEventMask);
        sq_pushinteger(vm, internalCollisionId);
        EntityUserData::eIDToUserData(vm, targetSenderId);

        return 5;
    }

    SQInteger populateFunc5(HSQUIRRELVM vm){
        sq_pushinteger(vm, targetCollisionUserId);
        sq_pushinteger(vm, (SQInteger)targetEventMask);
        sq_pushinteger(vm, internalCollisionId);
        EntityUserData::eIDToUserData(vm, targetSenderId);
        EntityUserData::eIDToUserData(vm, targetReceiverId);

        return 6;
    }

    void PhysicsCollisionDataManager::_processCollisionClosure(void* closureEntry, CollisionObjectEventMask::CollisionObjectEventMask eventMask, int senderId, int receiverId){
        if(closureEntry == INVALID_DATA_ID) return;
        const PhysicsCollisionDataManager::CollisionSenderClosureEntry& data = mSenderClosureObjects.getEntry(closureEntry);

        PopulateFunction populateFunction = 0;
        if(!_determinePopulateFunction(data.numParams, data.userData, eventMask, &populateFunction, senderId, receiverId)) return;

        ScriptVM::callClosure(data.closure, 0, populateFunction);
    }

    void PhysicsCollisionDataManager::_processCollisionScript(void* scriptEntry, CollisionObjectEventMask::CollisionObjectEventMask eventMask, int senderId, int receiverId){
        if(scriptEntry == INVALID_DATA_ID) return;
        const PhysicsCollisionDataManager::CollisionSenderScriptEntry& data = mSenderScriptObjects.getEntry(scriptEntry);

        //Invalid closure.
        if(data.closureId < 0) return;

        uint8 numParams = data.scriptPtr->getParamsForCallback(data.closureId);
        PopulateFunction populateFunction = 0;

        if(!_determinePopulateFunction(numParams, data.userData, eventMask, &populateFunction, senderId, receiverId)) return;

        data.scriptPtr->call(data.closureId, populateFunction);
    }

    void PhysicsCollisionDataManager::processCollision(const btCollisionObject* sender, const btCollisionObject* receiver, CollisionObjectEventMask::CollisionObjectEventMask eventMask){
        if(overrideFunction._type == OT_CLOSURE){
            //An override function is set, so rather than calling the actual function, call this instead.
            ScriptVM::callClosure(overrideFunction, 0, 0);
            return;
        }

        CollisionWorldUtils::PackedIntContents contents;
        CollisionWorldUtils::readPackedInt(sender->getUserIndex(), &contents);

        assert(contents.type != CollisionObjectType::RECEIVER);

        void* userPtr = sender->getUserPointer();
        int senderId = sender->getUserIndex3();
        int receiverId = receiver->getUserIndex3();
        if(userPtr == INVALID_DATA_ID) return;

        switch(contents.type){
            case CollisionObjectType::SENDER_SCRIPT:
                _processCollisionScript(userPtr, eventMask, senderId, receiverId);
                break;
            case CollisionObjectType::SENDER_CLOSURE:
                _processCollisionClosure(userPtr, eventMask, senderId, receiverId);
                break;
            default:
                assert(false);
                break;
        }
    }

    const PhysicsCollisionDataManager::CollisionSenderUserData* PhysicsCollisionDataManager::_getCollisionDataOfObject(const btCollisionObject* obj){
        void* pointerId = obj->getUserPointer();
        if(pointerId == INVALID_DATA_ID) return 0;
        CollisionWorldUtils::PackedIntContents contents;
        CollisionWorldUtils::readPackedInt(obj->getUserIndex(), &contents);

        const PhysicsCollisionDataManager::CollisionSenderUserData* ret = 0;
        if(contents.type == CollisionObjectType::SENDER_SCRIPT){
            ret = &mSenderScriptObjects.getEntry(pointerId).userData;
        }else{
            ret = &mSenderClosureObjects.getEntry(pointerId).userData;
        }
        return ret;
    }

    bool PhysicsCollisionDataManager::_determinePopulateFunction(uint8 numParams, const CollisionSenderUserData& data, CollisionObjectEventMask::CollisionObjectEventMask eventMask, PopulateFunction* outFunc, int senderId, int receiverId){
        switch(numParams){
            case 1:
                //Just an empty function call.
                break;
            case 2:
                *outFunc = &populateFunc1;
                targetCollisionUserId = data.userIndex;
                break;
            case 3:
                *outFunc = &populateFunc2;
                targetCollisionUserId = data.userIndex;
                targetEventMask = eventMask;
                break;
            case 4: {
                *outFunc = &populateFunc3;
                targetCollisionUserId = data.userIndex;
                targetEventMask = eventMask;
                const PhysicsMetaDataManager::PhysicsObjectMeta metaData = PhysicsMetaDataManager::getObjectMeta(senderId);
                internalCollisionId = metaData.id;
                break;
            }
            case 5: {
                *outFunc = &populateFunc4;
                targetCollisionUserId = data.userIndex;
                targetEventMask = eventMask;
                const PhysicsMetaDataManager::PhysicsObjectMeta metaData = PhysicsMetaDataManager::getObjectMeta(senderId);
                internalCollisionId = metaData.id;
                targetSenderId = metaData.attachedEntity;
                break;
            }
            case 6: {
                *outFunc = &populateFunc5;
                targetCollisionUserId = data.userIndex;
                targetEventMask = eventMask;
                const PhysicsMetaDataManager::PhysicsObjectMeta metaData = PhysicsMetaDataManager::getObjectMeta(senderId);
                internalCollisionId = metaData.id;
                targetSenderId = metaData.attachedEntity;
                const PhysicsMetaDataManager::PhysicsObjectMeta receiverMetaData = PhysicsMetaDataManager::getObjectMeta(receiverId);
                targetReceiverId = receiverMetaData.attachedEntity;
                break;
            }
            default:
                //Nothing to call as none of the functions matched up.
                return false;
        };

        return true;
    }

    void PhysicsCollisionDataManager::setCollisionCallbackOverride(SQObject closure){
        assert(closure._type == OT_CLOSURE || closure._type == OT_NULL);

        if(overrideFunction._type == OT_CLOSURE){
            ScriptVM::dereferenceObject(overrideFunction);
        }

        overrideFunction = closure;
    }
}
