#include "TestModePhysicsNamespace.h"

#include "System/BaseSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/PhysicsShapeManager.h"

namespace AV{
    SQInteger TestModePhysicsNamespace::getShapeExists(HSQUIRRELVM vm){
        SQInteger shapeType;
        SQFloat x, y, z;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);
        sq_getinteger(vm, -4, &shapeType);

        bool result = BaseSingleton::getPhysicsShapeManager()->shapeExists((PhysicsShapeManager::PhysicsShapeType)shapeType, btVector3(x, y, z));
        sq_pushbool(vm, result);
        return 1;
    }

    void TestModePhysicsNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        RedirectFunctionMap functionMap;
        functionMap["getShapeExists"] = {".nnnn", 5, getShapeExists};

        _redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}
