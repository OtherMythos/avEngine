#include "PhysicsShapeClass.h"

#include "World/Physics/PhysicsShapeManager.h"

namespace AV{
    PhysicsShapeClass::PhysicsShapeClass(){

    }

    PhysicsShapeClass::~PhysicsShapeClass(){

    }

    SQInteger PhysicsShapeClass::sqPhysicsShapeReleaseHook(SQUserPointer p, SQInteger size){
        PhysicsShapeManager *data = static_cast<PhysicsShapeManager*>(p);

        delete data;

        return 0;
    }

    void PhysicsShapeClass::createClassFromPointer(HSQUIRRELVM vm, PhysicsShapeManager::ShapePtr shape){
        void* ptr = sq_newuserdata(vm, sizeof(PhysicsShapeManager::ShapePtr));

        new (ptr)PhysicsShapeManager::ShapePtr(shape);

        sq_setreleasehook(vm, -1, sqPhysicsShapeReleaseHook);
    }

    void PhysicsShapeClass::setupClass(HSQUIRRELVM vm){

    }
}
