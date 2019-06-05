#include "PhysicsShapeClass.h"

#include "World/Physics/PhysicsShapeManager.h"

namespace AV{
    PhysicsShapeClass::PhysicsShapeClass(){

    }

    PhysicsShapeClass::~PhysicsShapeClass(){

    }

    SQInteger PhysicsShapeClass::sqPhysicsShapeReleaseHook(SQUserPointer p, SQInteger size){
        PhysicsShapeManager::ShapePtr *data = static_cast<PhysicsShapeManager::ShapePtr*>(p);

        data->reset();
        //I don't think this is actually needed. Regardless it was causing a crash.
        //delete data;

        return 0;
    }

    void PhysicsShapeClass::createClassFromPointer(HSQUIRRELVM vm, PhysicsShapeManager::ShapePtr shape){
        void* ptr = sq_newuserdata(vm, sizeof(PhysicsShapeManager::ShapePtr));
        PhysicsShapeManager::ShapePtr *shapePtr = static_cast<PhysicsShapeManager::ShapePtr*>(ptr);

        new (shapePtr)PhysicsShapeManager::ShapePtr;
        *shapePtr = shape;

        sq_setreleasehook(vm, -1, sqPhysicsShapeReleaseHook);
    }

    void PhysicsShapeClass::setupClass(HSQUIRRELVM vm){

    }
}
