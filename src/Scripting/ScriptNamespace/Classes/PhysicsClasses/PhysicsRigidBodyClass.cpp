#include "PhysicsRigidBodyClass.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

#include "PhysicsShapeClass.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"

#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "System/SystemSetup/SystemSettings.h"

namespace AV{
    SQObject PhysicsRigidBodyClass::classObject;
    DataPacker<PhysicsTypes::RigidBodyPtr> PhysicsRigidBodyClass::mBodyData;

    SQInteger PhysicsRigidBodyClass::rigidBodyCompare(HSQUIRRELVM vm){
        ASSERT_DYNAMIC_PHYSICS();
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
        ASSERT_DYNAMIC_PHYSICS();
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

            bool result = world->getPhysicsManager()->getDynamicsWorld()->bodyInWorld(body);

            sq_pushbool(vm, result);
        }

        return 1;
    }

    SQInteger PhysicsRigidBodyClass::bodyBoundType(HSQUIRRELVM vm){
        ASSERT_DYNAMIC_PHYSICS();
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

            SQInteger retVal = (SQInteger) world->getPhysicsManager()->getDynamicsWorld()->getBodyBindType(body);

            sq_pushinteger(vm, retVal);
        }

        return 1;
    }

    SQInteger PhysicsRigidBodyClass::getBodyShape(HSQUIRRELVM vm){
        ASSERT_DYNAMIC_PHYSICS();
        PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

        PhysicsTypes::ShapePtr shape = PhysicsBodyConstructor::getBodyShape(body.get());

        PhysicsShapeClass::createInstanceFromPointer(vm, shape);

        return 1;
    }

    SQInteger PhysicsRigidBodyClass::setBodyPosition(HSQUIRRELVM vm){
        ASSERT_DYNAMIC_PHYSICS();
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -2);

            Ogre::Vector3 pos;
            SQInteger result = ScriptGetterUtils::vector3Read(vm, &pos);
            if(result != 0) return result;

            world->getPhysicsManager()->getDynamicsWorld()->setBodyPosition(body, OGRE_TO_BULLET(pos));
        }

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::setLinearFactor(HSQUIRRELVM vm){
        ASSERT_DYNAMIC_PHYSICS();
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -2);

            Ogre::Vector3 outVal;
            SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &outVal));

            world->getPhysicsManager()->getDynamicsWorld()->setBodyLinearFactor(body, OGRE_TO_BULLET(outVal));
        }

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::setLinearVelocity(HSQUIRRELVM vm){
        ASSERT_DYNAMIC_PHYSICS();
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -2);

            Ogre::Vector3 outVal;
            SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &outVal));

            world->getPhysicsManager()->getDynamicsWorld()->setBodyLinearVelocity(body, OGRE_TO_BULLET(outVal));
        }

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::sqPhysicsRigidBodyReleaseHook(SQUserPointer p, SQInteger size){
        mBodyData.getEntry(p).reset();

        mBodyData.removeEntry(p);

        return 0;
    }

    SQInteger PhysicsRigidBodyClass::getBodyPosition(HSQUIRRELVM vm){
        ASSERT_DYNAMIC_PHYSICS();
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, 1);

            btVector3 pos = world->getPhysicsManager()->getDynamicsWorld()->getBodyPosition(body);

            Vector3UserData::vector3ToUserData(vm, Ogre::Vector3(pos));
        }

        return 1;
    }

    SQInteger PhysicsRigidBodyClass::getBodyLinearVelocity(HSQUIRRELVM vm){
        ASSERT_DYNAMIC_PHYSICS();
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, 1);

            btVector3 vel = world->getPhysicsManager()->getDynamicsWorld()->getBodyLinearVelocity(body);

            Vector3UserData::vector3ToUserData(vm, Ogre::Vector3(vel));
        }
        return 1;
    }

    SQInteger PhysicsRigidBodyClass::getInternalId(HSQUIRRELVM vm){
        ASSERT_DYNAMIC_PHYSICS();
        //We don't need the world here as the objects and ids exist outside of it.

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, 1);

            CollisionInternalId vel = DynamicsWorld::getBodyInternalIdStatic(body);
            sq_pushinteger(vm, vel);
        }

        return 1;
    }

    void PhysicsRigidBodyClass::createInstanceFromPointer(HSQUIRRELVM vm, PhysicsTypes::RigidBodyPtr body){
        ASSERT_DYNAMIC_PHYSICS();
        sq_pushobject(vm, classObject);

        sq_createinstance(vm, -1);

        void* id = mBodyData.storeEntry(body);
        sq_setinstanceup(vm, -1, (SQUserPointer*)id);

        sq_setreleasehook(vm, -1, sqPhysicsRigidBodyReleaseHook);
    }

    PhysicsTypes::RigidBodyPtr PhysicsRigidBodyClass::getRigidBodyFromInstance(HSQUIRRELVM vm, SQInteger index){
        ASSERT_DYNAMIC_PHYSICS();
        SQUserPointer p;
        sq_getinstanceup(vm, index, &p, 0, false);

        return mBodyData.getEntry(p);
    }

    void PhysicsRigidBodyClass::setupClass(HSQUIRRELVM vm){
        if(SystemSettings::getDynamicPhysicsDisabled()) return;

        sq_newclass(vm, 0);

        ScriptUtils::addFunction(vm, bodyInWorld, "inWorld");
        ScriptUtils::addFunction(vm, bodyBoundType, "boundType");
        ScriptUtils::addFunction(vm, getBodyShape, "getShape");
        ScriptUtils::addFunction(vm, setBodyPosition, "setPosition", -2, ".n|unn");
        ScriptUtils::addFunction(vm, setLinearFactor, "setLinearFactor", -2, ".n|unn");
        ScriptUtils::addFunction(vm, setLinearVelocity, "setLinearVelocity", -2, ".n|unn");
        ScriptUtils::addFunction(vm, getBodyPosition, "getPosition");
        ScriptUtils::addFunction(vm, getBodyLinearVelocity, "getLinearVelocity");
        ScriptUtils::addFunction(vm, getInternalId, "getInternalId");
        ScriptUtils::addFunction(vm, rigidBodyCompare, "_cmp");

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }
}
