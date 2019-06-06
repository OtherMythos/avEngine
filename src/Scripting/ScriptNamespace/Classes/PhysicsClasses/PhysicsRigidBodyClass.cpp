#include "PhysicsRigidBodyClass.h"

namespace AV{
    SQObject PhysicsRigidBodyClass::classObject;
    
    PhysicsRigidBodyClass::PhysicsRigidBodyClass(){

    }

    PhysicsRigidBodyClass::~PhysicsRigidBodyClass(){

    }

    SQInteger PhysicsRigidBodyClass::bodyValid(HSQUIRRELVM vm){
        return 0;
    }

    void PhysicsRigidBodyClass::setupClass(HSQUIRRELVM vm){
        //I'll want to save this as an object or somewhere in the reference table rather than in the root table.
        sq_pushstring(vm, _SC("RigidBody"), -1);
        sq_newclass(vm, 0);

        sq_pushstring(vm, _SC("valid"), -1);
        sq_newclosure(vm, bodyValid, 0);
        sq_newslot(vm, -3, false);

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);

        sq_newslot(vm, -3, false);
    }
}
