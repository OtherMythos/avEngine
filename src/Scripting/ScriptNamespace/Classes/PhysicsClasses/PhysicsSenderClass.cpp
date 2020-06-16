#include "PhysicsSenderClass.h"

namespace AV{

    ScriptDataPacker<PhysicsTypes::CollisionSenderPtr> PhysicsSenderClass::mObjectData;
    SQObject PhysicsSenderClass::classObject;

    void PhysicsSenderClass::setupClass(HSQUIRRELVM vm){
        sq_newclass(vm, 0);

        //ScriptUtils::addFunction(vm, physicsShapeCompare, "_cmp");

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }

    void PhysicsSenderClass::createInstanceFromPointer(HSQUIRRELVM vm, PhysicsTypes::CollisionSenderPtr shape){
        sq_pushobject(vm, classObject);

        sq_createinstance(vm, -1);

        void* id = mObjectData.storeEntry(shape);
        sq_setinstanceup(vm, -1, (SQUserPointer*)id);

        sq_setreleasehook(vm, -1, physicsSenderReleaseHook);

        //Remove the class object.
        sq_remove(vm, -2);
    }

    bool PhysicsSenderClass::getPointerFromInstance(HSQUIRRELVM vm, SQInteger index, PhysicsTypes::CollisionSenderPtr* outPtr){
        SQUserPointer p, typetag;
        sq_getinstanceup(vm, index, &p, &typetag);

        //TODO check the typetag at some point.
        *outPtr = mObjectData.getEntry(p);

        return true;
    }

    SQInteger PhysicsSenderClass::physicsSenderReleaseHook(SQUserPointer p, SQInteger size){
        mObjectData.getEntry(p).reset();

        mObjectData.removeEntry(p);

        return 0;
    }
}
