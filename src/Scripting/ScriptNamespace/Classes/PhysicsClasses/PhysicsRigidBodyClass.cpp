#include "PhysicsRigidBodyClass.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

#include "PhysicsShapeClass.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"

#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"

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
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

            bool result = world->getPhysicsManager()->getDynamicsWorld()->bodyInWorld(body);

            sq_pushbool(vm, result);
        }

        return 1;
    }

    SQInteger PhysicsRigidBodyClass::bodyBoundType(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

            SQInteger retVal = (SQInteger) world->getPhysicsManager()->getDynamicsWorld()->getBodyBindType(body);

            sq_pushinteger(vm, retVal);
        }

        return 1;
    }

    SQInteger PhysicsRigidBodyClass::getBodyShape(HSQUIRRELVM vm){
        PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

        PhysicsTypes::ShapePtr shape = PhysicsBodyConstructor::getBodyShape(body.get());

        PhysicsShapeClass::createInstanceFromPointer(vm, shape);

        return 1;
    }

    SQInteger PhysicsRigidBodyClass::setBodyPosition(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -2);

            Ogre::Vector3 outVec;
            if(!ScriptGetterUtils::vector3Read(vm, &outVec)) return sq_throwerror(vm, "Invalid object provided.");

            world->getPhysicsManager()->getDynamicsWorld()->setBodyPosition(body, btVector3(outVec.x, outVec.y, outVec.z));
        }

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::setLinearFactor(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -2);

            Ogre::Vector3 outVal;
            if(!Vector3UserData::readVector3FromUserData(vm, -1, &outVal)) return sq_throwerror(vm, "Invalid object provided.");

            world->getPhysicsManager()->getDynamicsWorld()->setBodyLinearFactor(body, btVector3(outVal.x, outVal.y, outVal.z));
        }

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::setLinearVelocity(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -2);

            Ogre::Vector3 outVal;
            if(!Vector3UserData::readVector3FromUserData(vm, -1, &outVal)) return sq_throwerror(vm, "Invalid object provided.");

            world->getPhysicsManager()->getDynamicsWorld()->setBodyLinearVelocity(body, btVector3(outVal.x, outVal.y, outVal.z));
        }

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::sqPhysicsRigidBodyReleaseHook(SQUserPointer p, SQInteger size){
        mBodyData.getEntry(p).reset();

        mBodyData.removeEntry(p);

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::getBodyPosition(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, 1);

            btVector3 pos = world->getPhysicsManager()->getDynamicsWorld()->getBodyPosition(body);

            Vector3UserData::vector3ToUserData(vm, Ogre::Vector3(pos));
        }

        return 1;
    }

    SQInteger PhysicsRigidBodyClass::getBodyLinearVelocity(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, 1);

            btVector3 vel = world->getPhysicsManager()->getDynamicsWorld()->getBodyLinearVelocity(body);

            Vector3UserData::vector3ToUserData(vm, Ogre::Vector3(vel));
        }
        return 1;
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
        ScriptUtils::addFunction(vm, setBodyPosition, "setPosition", -2, ".n|unn");
        ScriptUtils::addFunction(vm, setLinearFactor, "setLinearFactor", 2, ".u");
        ScriptUtils::addFunction(vm, setLinearVelocity, "setLinearVelocity", 2, ".u");
        ScriptUtils::addFunction(vm, getBodyPosition, "getPosition");
        ScriptUtils::addFunction(vm, getBodyLinearVelocity, "getLinearVelocity");
        ScriptUtils::addFunction(vm, rigidBodyCompare, "_cmp");

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }
}
