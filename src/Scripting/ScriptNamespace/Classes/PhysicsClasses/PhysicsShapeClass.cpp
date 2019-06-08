#include "PhysicsShapeClass.h"

#include "World/Physics/PhysicsShapeManager.h"

namespace AV{

    ScriptDataPacker<PhysicsShapeManager::ShapePtr> PhysicsShapeClass::mShapeData;
    SQObject PhysicsShapeClass::classObject;

    PhysicsShapeClass::PhysicsShapeClass(){

    }

    PhysicsShapeClass::~PhysicsShapeClass(){

    }

    SQInteger PhysicsShapeClass::physicsShapeCompare(HSQUIRRELVM vm){
        SQUserPointer p, q;
        sq_getinstanceup(vm, -1, &p, 0);
        sq_getinstanceup(vm, -2, &q, 0);

        if(mShapeData.getEntry(p) == mShapeData.getEntry(q)){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushbool(vm, false);
        }
        return 1;
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

        sq_pushstring(vm, _SC("_cmp"), -1);
        sq_newclosure(vm, physicsShapeCompare, 0);
        sq_newslot(vm, -3, false);

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }
}
