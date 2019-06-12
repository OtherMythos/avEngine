#include "PhysicsRigidBodyClass.h"

namespace AV{
    SQObject PhysicsRigidBodyClass::classObject;
    ScriptDataPacker<PhysicsRigidBodyClass::RigidBodyInfo> PhysicsRigidBodyClass::mBodyData;

    PhysicsRigidBodyClass::PhysicsRigidBodyClass(){

    }

    PhysicsRigidBodyClass::~PhysicsRigidBodyClass(){

    }

    SQInteger PhysicsRigidBodyClass::bodyValid(HSQUIRRELVM vm){
        return 0;
    }

    SQInteger PhysicsRigidBodyClass::sqPhysicsRigidBodyReleaseHook(SQUserPointer p, SQInteger size){
        mBodyData.getEntry(p).shapePtr.reset();

        mBodyData.removeEntry(p);

        return 0;
    }

    void PhysicsRigidBodyClass::_createInstanceFromInfo(HSQUIRRELVM vm, DynamicsWorld::RigidBodyPtr body, PhysicsShapeManager::ShapePtr shapePtr){
        sq_pushobject(vm, classObject);

        sq_createinstance(vm, -1);

        //We need to store the shape ptr as well.
        //The object is using it, so we need to make sure nothing happens to it in the mean time (i.e the references reach 0).
        RigidBodyInfo i = {body, shapePtr};
        void* id = mBodyData.storeEntry(i);
        sq_setinstanceup(vm, -1, (SQUserPointer*)id);

        sq_setreleasehook(vm, -1, sqPhysicsRigidBodyReleaseHook);
    }

    DynamicsWorld::RigidBodyPtr PhysicsRigidBodyClass::getRigidBodyFromInstance(HSQUIRRELVM vm, SQInteger index){
        SQUserPointer p;
        sq_getinstanceup(vm, index, &p, 0);

        const RigidBodyInfo& info = mBodyData.getEntry(p);

        return info.body;
    }

    void PhysicsRigidBodyClass::setupClass(HSQUIRRELVM vm){
        sq_newclass(vm, 0);

        sq_pushstring(vm, _SC("valid"), -1);
        sq_newclosure(vm, bodyValid, 0);
        sq_newslot(vm, -3, false);

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }
}
