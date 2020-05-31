#include "PhysicsRigidBodyClass.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

#include "PhysicsShapeClass.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"

namespace AV{
    SQObject PhysicsRigidBodyClass::classObject;
    ScriptDataPacker<PhysicsTypes::RigidBodyPtr> PhysicsRigidBodyClass::mBodyData;

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

    SQInteger PhysicsRigidBodyClass::setLinearFactor(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -4);

            SQFloat sx, sy, sz;
            sq_getfloat(vm, -1, &sz);
            sq_getfloat(vm, -2, &sy);
            sq_getfloat(vm, -3, &sx);

            world->getPhysicsManager()->getDynamicsWorld()->setBodyLinearFactor(body, btVector3(sx, sy, sz));
        }

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::setLinearVelocity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -4);

            SQFloat sx, sy, sz;
            sq_getfloat(vm, -1, &sz);
            sq_getfloat(vm, -2, &sy);
            sq_getfloat(vm, -3, &sx);

            world->getPhysicsManager()->getDynamicsWorld()->setBodyLinearVelocity(body, btVector3(sx, sy, sz));
        }

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::sqPhysicsRigidBodyReleaseHook(SQUserPointer p, SQInteger size){
        mBodyData.getEntry(p).reset();

        mBodyData.removeEntry(p);

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::getBodyPosition(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, 1);

            btVector3 pos = world->getPhysicsManager()->getDynamicsWorld()->getBodyPosition(body);

            //TODO this is copy and pasted from the SlotPosition class.
            //I need to figure out what the most efficient way to store a vector 3 is, and start using that more.
            sq_newarray(vm, 3);
            sq_pushfloat(vm, pos.z());
            sq_pushfloat(vm, pos.y());
            sq_pushfloat(vm, pos.x());
            sq_arrayinsert(vm, -4, 0);
            sq_arrayinsert(vm, -3, 1);
            sq_arrayinsert(vm, -2, 2);

            return 1;
        }

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::getBodyLinearVelocity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, 1);

            btVector3 vel = world->getPhysicsManager()->getDynamicsWorld()->getBodyLinearVelocity(body);

            //TODO Same as above. Also remove the copy and pasting.
            sq_newarray(vm, 3);
            sq_pushfloat(vm, vel.z());
            sq_pushfloat(vm, vel.y());
            sq_pushfloat(vm, vel.x());
            sq_arrayinsert(vm, -4, 0);
            sq_arrayinsert(vm, -3, 1);
            sq_arrayinsert(vm, -2, 2);

            return 1;
        }

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

        ScriptUtils::addFunction(vm, bodyInWorld, "inWorld");
        ScriptUtils::addFunction(vm, bodyBoundType, "boundType");
        ScriptUtils::addFunction(vm, getBodyShape, "getShape");
        ScriptUtils::addFunction(vm, setBodyPosition, "setPosition");
        ScriptUtils::addFunction(vm, setLinearFactor, "setLinearFactor");
        ScriptUtils::addFunction(vm, setLinearVelocity, "setLinearVelocity");
        ScriptUtils::addFunction(vm, getBodyPosition, "getPosition");
        ScriptUtils::addFunction(vm, getBodyLinearVelocity, "getLinearVelocity");
        ScriptUtils::addFunction(vm, rigidBodyCompare, "_cmp");

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }
}
