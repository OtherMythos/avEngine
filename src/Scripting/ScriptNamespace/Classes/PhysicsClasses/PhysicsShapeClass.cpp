#include "PhysicsShapeClass.h"

#include "World/Physics/PhysicsShapeManager.h"

namespace AV{

    ScriptDataPacker<PhysicsShapeManager::ShapePtr> PhysicsShapeClass::mShapeData;
    SQObject PhysicsShapeClass::classObject;

    PhysicsShapeClass::PhysicsShapeClass(){

    }

    PhysicsShapeClass::~PhysicsShapeClass(){

    }

    SQInteger PhysicsShapeClass::sqPhysicsShapeReleaseHook(SQUserPointer p, SQInteger size){
        //Remove the reference to the shape.
        mShapeData.getEntry(p).reset();

        mShapeData.removeEntry(p);

        return 0;
    }

    void PhysicsShapeClass::createClassFromPointer(HSQUIRRELVM vm, PhysicsShapeManager::ShapePtr shape){
        sq_pushobject(vm, classObject);

        sq_createinstance(vm, -1);

        void* id = mShapeData.storeEntry(shape);
        sq_setinstanceup(vm, -1, (SQUserPointer*)id);

        sq_setreleasehook(vm, -1, sqPhysicsShapeReleaseHook);
    }

    void PhysicsShapeClass::setupClass(HSQUIRRELVM vm){
        sq_newclass(vm, 0);

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }
}
