#include "TestModePhysicsNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/PhysicsShapeManager.h"

namespace AV{
    SQInteger TestModePhysicsNamespace::getShapeExists(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(world){
            SQInteger shapeType;
            SQFloat x, y, z;
            sq_getfloat(vm, -1, &z);
            sq_getfloat(vm, -2, &y);
            sq_getfloat(vm, -3, &x);
            sq_getinteger(vm, -4, &shapeType);

            bool result = world->getPhysicsManager()->getShapeManager()->shapeExists((PhysicsShapeManager::PhysicsShapeType)shapeType, btVector3(x, y, z));
            sq_pushbool(vm, result);
            return 1;
        }
        return 0;
    }

    void TestModePhysicsNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        RedirectFunctionMap functionMap;
        functionMap["getShapeExists"] = {".nnnn", 5, getShapeExists};

        _redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}
