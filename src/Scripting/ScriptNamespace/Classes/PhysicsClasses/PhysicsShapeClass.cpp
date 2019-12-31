#include "PhysicsShapeClass.h"

#include "World/Physics/PhysicsShapeManager.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    ScriptDataPacker<PhysicsTypes::ShapePtr> PhysicsShapeClass::mShapeData;
    SQObject PhysicsShapeClass::classObject;

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

    void PhysicsShapeClass::createInstanceFromPointer(HSQUIRRELVM vm, PhysicsTypes::ShapePtr shape){
        sq_pushobject(vm, classObject);

        sq_createinstance(vm, -1);

        void* id = mShapeData.storeEntry(shape);
        sq_setinstanceup(vm, -1, (SQUserPointer*)id);

        sq_setreleasehook(vm, -1, sqPhysicsShapeReleaseHook);
    }

    PhysicsTypes::ShapePtr PhysicsShapeClass::getPointerFromInstance(HSQUIRRELVM vm, SQInteger index){
        SQUserPointer p;
        sq_getinstanceup(vm, index, &p, 0);

        return mShapeData.getEntry(p);
    }

    void PhysicsShapeClass::setupClass(HSQUIRRELVM vm){
        sq_newclass(vm, 0);

        ScriptUtils::addFunction(vm, physicsShapeCompare, "_cmp");

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }
}
