#include "PhysicsNamespace.h"

#include "World/WorldSingleton.h"

namespace AV {
    SQInteger PhysicsNamespace::obtainCubeShape(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SQFloat x, y, z;
            sq_getfloat(vm, -1, &z);
            sq_getfloat(vm, -2, &y);
            sq_getfloat(vm, -3, &x);
            
            
            
            return 1;
        }
        return 0;
    }
    
    void PhysicsNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, obtainCubeShape, "obtainCubeShape", 4, ".nnn");
    }
}
