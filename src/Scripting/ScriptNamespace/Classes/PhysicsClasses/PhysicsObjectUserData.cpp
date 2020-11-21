#include "PhysicsObjectUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/CollisionWorld.h"

#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "System/EnginePrerequisites.h"
#include "System/SystemSetup/SystemSettings.h"

namespace AV{

    SQObject PhysicsObjectUserData::senderDelegateTable;
    SQObject PhysicsObjectUserData::receiverDelegateTable;

    void PhysicsObjectUserData::setupDelegateTable(HSQUIRRELVM vm){
        if(SystemSettings::getNumCollisionWorlds() <= 0) return;

        { //Sender delegate table
            sq_newtableex(vm, 1);

            ScriptUtils::addFunction(vm, setObjectPosition, "setPosition", -2, ".n|unn");
            ScriptUtils::addFunction(vm, getUserIndex, "getUserIndex", 1, ".u");

            sq_resetobject(&senderDelegateTable);
            sq_getstackobj(vm, -1, &senderDelegateTable);
            sq_addref(vm, &senderDelegateTable);
            sq_pop(vm, 1);
        }

        { //Receiver delegate table
            sq_newtableex(vm, 1);

            ScriptUtils::addFunction(vm, setObjectPosition, "setPosition", -2, ".n|unn");

            sq_resetobject(&receiverDelegateTable);
            sq_getstackobj(vm, -1, &receiverDelegateTable);
            sq_addref(vm, &receiverDelegateTable);
            sq_pop(vm, 1);
        }
    }

    SQInteger PhysicsObjectUserData::getUserIndex(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::CollisionObjectPtr targetObject;
            SCRIPT_CHECK_RESULT(getPointerFromUserData(vm, 1, &targetObject, SENDER));

            int outIdx = 0;
            CollisionWorld::CollisionFunctionStatus ret = CollisionWorld::getUserIndexStatic(targetObject, &outIdx);
            assert(ret == CollisionWorld::CollisionFunctionStatus::SUCCESS);

            sq_pushinteger(vm, outIdx);
        }

        return 1;
    }

    SQInteger PhysicsObjectUserData::setObjectPosition(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::CollisionObjectPtr targetObject;
            SCRIPT_CHECK_RESULT(getPointerFromUserData(vm, 1, &targetObject, EITHER));

            Ogre::Vector3 outVec;
            if(!ScriptGetterUtils::vector3Read(vm, &outVec)) return sq_throwerror(vm, "Invalid object provided.");

            CollisionWorld::setObjectPositionStatic(targetObject, OGRE_TO_BULLET(outVec));
        }

        return 0;
    }

    void PhysicsObjectUserData::collisionObjectFromPointer(HSQUIRRELVM vm, PhysicsTypes::CollisionObjectPtr object, bool receiver){
        PhysicsTypes::CollisionObjectPtr* pointer = (PhysicsTypes::CollisionObjectPtr*)sq_newuserdata(vm, sizeof(PhysicsTypes::CollisionObjectPtr));
        new (pointer) PhysicsTypes::CollisionObjectPtr(object);

        sq_pushobject(vm, receiver ? receiverDelegateTable : senderDelegateTable);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, receiver ? CollisionReceiverTypeTag : CollisionSenderTypeTag);
        sq_setreleasehook(vm, -1, physicsObjectReleaseHook);
    }

    UserDataGetResult PhysicsObjectUserData::getPointerFromUserData(HSQUIRRELVM vm, SQInteger index, PhysicsTypes::CollisionObjectPtr* outPtr, GetCollisionObjectType getType){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, index, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(getType == EITHER){
            if(typeTag != CollisionReceiverTypeTag && typeTag != CollisionSenderTypeTag){
                return USER_DATA_GET_TYPE_MISMATCH;
            }
        }else{
            if(typeTag != (getType == RECEIVER ? CollisionReceiverTypeTag : CollisionSenderTypeTag) ){
                return USER_DATA_GET_TYPE_MISMATCH;
            }
        }

        *outPtr = *((PhysicsTypes::CollisionObjectPtr*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger PhysicsObjectUserData::physicsObjectReleaseHook(SQUserPointer p, SQInteger size){
        PhysicsTypes::CollisionObjectPtr* ptr = static_cast<PhysicsTypes::CollisionObjectPtr*>(p);
        ptr->reset();
        //I don't actually think I need to call reset here. The memory will eventually be returned. All that matters is the reference is destroyed.

        return 0;
    }
}
