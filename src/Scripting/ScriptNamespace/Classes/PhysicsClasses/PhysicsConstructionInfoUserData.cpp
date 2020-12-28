#include "PhysicsConstructionInfoUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    void PhysicsConstructionInfoUserData::dynamicConstructionInfoFromData(HSQUIRRELVM vm, const btRigidBody::btRigidBodyConstructionInfo& info){
        btRigidBody::btRigidBodyConstructionInfo* pointer = (btRigidBody::btRigidBodyConstructionInfo*)sq_newuserdata(vm, sizeof(btRigidBody::btRigidBodyConstructionInfo));
        new (pointer) btRigidBody::btRigidBodyConstructionInfo(info);

        sq_settypetag(vm, -1, DynamicsConstructionInfoTypeTag);
    }

    UserDataGetResult PhysicsConstructionInfoUserData::getDynamicConstructionInfo(HSQUIRRELVM vm, SQInteger index, btRigidBody::btRigidBodyConstructionInfo& outInfo){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, index, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != DynamicsConstructionInfoTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        outInfo = *((btRigidBody::btRigidBodyConstructionInfo*)pointer);

        return USER_DATA_GET_SUCCESS;
    }
}
