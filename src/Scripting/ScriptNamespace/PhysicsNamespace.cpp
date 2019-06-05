#include "PhysicsNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/PhysicsShapeManager.h"

#include "Scripting/ScriptNamespace/Classes/PhysicsClasses/PhysicsShapeClass.h"

namespace AV {
    SQInteger PhysicsNamespace::getCubeShape(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SQFloat x, y, z;
            sq_getfloat(vm, -1, &z);
            sq_getfloat(vm, -2, &y);
            sq_getfloat(vm, -3, &x);

            PhysicsShapeManager::ShapePtr shape =
                world->getPhysicsManager()->getShapeManager()->getBoxShape(btVector3(x, y, z));
            PhysicsShapeClass::createClassFromPointer(vm, shape);

            return 1;
        }
        return 0;
    }

    SQInteger PhysicsNamespace::getSphereShape(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SQFloat radius;
            sq_getfloat(vm, -1, &radius);

            PhysicsShapeManager::ShapePtr shape =
                world->getPhysicsManager()->getShapeManager()->getSphereShape(radius);
            PhysicsShapeClass::createClassFromPointer(vm, shape);

            return 1;
        }
        return 0;
    }

    void PhysicsNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, getCubeShape, "getCubeShape", 4, ".nnn");
        _addFunction(vm, getSphereShape, "getSphereShape", 2, ".n");
    }
}
