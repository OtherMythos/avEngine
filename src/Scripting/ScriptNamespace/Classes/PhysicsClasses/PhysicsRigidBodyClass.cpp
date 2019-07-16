#include "PhysicsRigidBodyClass.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

#include "PhysicsShapeClass.h"

#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"

namespace AV{
    SQObject PhysicsRigidBodyClass::classObject;
    ScriptDataPacker<PhysicsTypes::RigidBodyPtr> PhysicsRigidBodyClass::mBodyData;

    PhysicsRigidBodyClass::PhysicsRigidBodyClass(){

    }

    PhysicsRigidBodyClass::~PhysicsRigidBodyClass(){

    }

    SQInteger PhysicsRigidBodyClass::rigidBodyCompare(HSQUIRRELVM vm){
        PhysicsTypes::RigidBodyPtr first = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);
        PhysicsTypes::RigidBodyPtr second = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -2);

        if(first == second){
            sq_pushinteger(vm, 0);
        }else{
            //Any number other than the ones specified in the documentation equate to not equal.
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    SQInteger PhysicsRigidBodyClass::bodyInWorld(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

            world->getPhysicsManager()->getDynamicsWorld()->bodyInWorld(body);
        }

        sq_pushbool(vm, false);
        return 1;
    }

    SQInteger PhysicsRigidBodyClass::bodyBoundType(HSQUIRRELVM vm){
        PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

        World *world = WorldSingleton::getWorld();
        SQInteger retVal = 0;
        if(world){
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

            retVal = (SQInteger) world->getPhysicsManager()->getDynamicsWorld()->getBodyBindType(body);
        }

        sq_pushinteger(vm, retVal);
        return 1;
    }

    SQInteger PhysicsRigidBodyClass::getBodyShape(HSQUIRRELVM vm){
        PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

        PhysicsTypes::ShapePtr shape = PhysicsBodyConstructor::getBodyShape(body.get());

        PhysicsShapeClass::createInstanceFromPointer(vm, shape);

        return 1;
    }

    SQInteger PhysicsRigidBodyClass::setBodyPosition(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -2);

            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);

            world->getPhysicsManager()->getDynamicsWorld()->setBodyPosition(body, pos.toBullet());
        }

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::sqPhysicsRigidBodyReleaseHook(SQUserPointer p, SQInteger size){
        mBodyData.getEntry(p).reset();

        mBodyData.removeEntry(p);

        return 0;
    }

    void PhysicsRigidBodyClass::createInstanceFromPointer(HSQUIRRELVM vm, PhysicsTypes::RigidBodyPtr body){
        sq_pushobject(vm, classObject);

        sq_createinstance(vm, -1);

        void* id = mBodyData.storeEntry(body);
        sq_setinstanceup(vm, -1, (SQUserPointer*)id);

        sq_setreleasehook(vm, -1, sqPhysicsRigidBodyReleaseHook);
    }

    PhysicsTypes::RigidBodyPtr PhysicsRigidBodyClass::getRigidBodyFromInstance(HSQUIRRELVM vm, SQInteger index){
        SQUserPointer p;
        sq_getinstanceup(vm, index, &p, 0);

        return mBodyData.getEntry(p);
    }

    void PhysicsRigidBodyClass::setupClass(HSQUIRRELVM vm){
        sq_newclass(vm, 0);

        sq_pushstring(vm, _SC("inWorld"), -1);
        sq_newclosure(vm, bodyInWorld, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("boundType"), -1);
        sq_newclosure(vm, bodyBoundType, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("getShape"), -1);
        sq_newclosure(vm, getBodyShape, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setPosition"), -1);
        sq_newclosure(vm, setBodyPosition, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("_cmp"), -1);
        sq_newclosure(vm, rigidBodyCompare, 0);
        sq_newslot(vm, -3, false);

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }
}
