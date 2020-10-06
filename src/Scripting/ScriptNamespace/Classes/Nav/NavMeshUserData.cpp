#include "NavMeshUserData.h"

#include "World/WorldSingleton.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    SQObject NavMeshUserData::meshDelegateTable;

    void NavMeshUserData::meshToUserData(HSQUIRRELVM vm, NavMeshId mesh){
        NavMeshId* pointer = (NavMeshId*)sq_newuserdata(vm, sizeof(NavMeshId));
        *pointer = mesh;

        sq_pushobject(vm, meshDelegateTable);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, NavMeshTypeTag);
    }

    UserDataGetResult NavMeshUserData::readMeshFromUserData(HSQUIRRELVM vm, SQInteger stackInx, NavMeshId* outMesh){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != NavMeshTypeTag){
            *outMesh = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        NavMeshId* p = static_cast<NavMeshId*>(pointer);
        *outMesh = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger NavMeshUserData::testRay(HSQUIRRELVM vm){

        return 0;
    }

    void NavMeshUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newclass(vm, 0);

        ScriptUtils::addFunction(vm, testRay, "testRay", 2, ".u");

        sq_settypetag(vm, -1, NavMeshTypeTag);
        sq_resetobject(&meshDelegateTable);
        sq_getstackobj(vm, -1, &meshDelegateTable);
        sq_addref(vm, &meshDelegateTable);
        sq_pop(vm, 1);
    }
}
