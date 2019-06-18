#include "PhysicsNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

#include "btBulletDynamicsCommon.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/Classes/PhysicsClasses/PhysicsShapeClass.h"
#include "Scripting/ScriptNamespace/Classes/PhysicsClasses/PhysicsRigidBodyClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV {
    SQInteger PhysicsNamespace::getCubeShape(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SQFloat x, y, z;
            sq_getfloat(vm, -1, &z);
            sq_getfloat(vm, -2, &y);
            sq_getfloat(vm, -3, &x);

            PhysicsShapeManager::ShapePtr shape =
                world->getPhysicsManager()->getShapeManager()->getBoxShape(btVector3(x, y, z));
            PhysicsShapeClass::createInstanceFromPointer(vm, shape);

            return 1;
        }
        return 0;
    }

    SQInteger PhysicsNamespace::getSphereShape(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SQFloat radius;
            sq_getfloat(vm, -1, &radius);

            PhysicsShapeManager::ShapePtr shape =
                world->getPhysicsManager()->getShapeManager()->getSphereShape(radius);
            PhysicsShapeClass::createInstanceFromPointer(vm, shape);

            return 1;
        }
        return 0;
    }

    void PhysicsNamespace::_iterateConstructionInfoTable(HSQUIRRELVM vm, SQInteger tableIndex, btRigidBody::btRigidBodyConstructionInfo& info){
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm,-2))){
            //here -1 is the value and -2 is the key
            const SQChar *k;
            sq_getstring(vm, -2, &k);
            std::string key(k);

            SQObjectType t = sq_gettype(vm, -1);
            if(t == OT_FLOAT || t == OT_INTEGER){
                SQFloat val;
                sq_getfloat(vm, -1, &val);

                if(key == "mass"){
                    info.m_mass = val;
                }
                else if(key == "friction"){
                    info.m_friction = val;
                }
                else if(key == "rollingFriction"){
                    info.m_rollingFriction = val;
                }
                else if(key == "spinningFriction"){
                    info.m_spinningFriction = val;
                }
                else if(key == "restitution"){
                    info.m_restitution = val;
                }
            }else if(t == OT_ARRAY){
                if(key == "origin"){
                    SQFloat vals[3];
                    ScriptUtils::getFloatArray<3>(vm, vals);

                    info.m_startWorldTransform.setOrigin(btVector3(vals[0], vals[1], vals[2]));
                }
                else if(key == "rotation"){
                    SQFloat vals[4];
                    ScriptUtils::getFloatArray<4>(vm, vals);

                    info.m_startWorldTransform.setRotation(btQuaternion(vals[0], vals[1], vals[2], vals[3]));
                }
            }

            sq_pop(vm,2); //pop the key and value
        }

        sq_pop(vm,1); //pops the null iterator
    }

    SQInteger PhysicsNamespace::createRigidBody(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            btRigidBody::btRigidBodyConstructionInfo rbInfo(1, 0, 0);
            PhysicsShapeManager::ShapePtr shape;

            rbInfo.m_startWorldTransform.setIdentity();

            SQInteger nargs = sq_gettop(vm);
            if(nargs == 3){
                _iterateConstructionInfoTable(vm, -1, rbInfo);
                shape = PhysicsShapeClass::getPointerFromInstance(vm, -2);
            }else if(nargs == 2){
                //Just a shape
                shape = PhysicsShapeClass::getPointerFromInstance(vm, -1);
            }

            btVector3 localInertia(0, 0, 0);
            if(rbInfo.m_mass != 0.0f){
                shape.get()->calculateLocalInertia(rbInfo.m_mass, localInertia);
                rbInfo.m_localInertia = localInertia;
            }

            DynamicsWorld::RigidBodyPtr body = world->getPhysicsManager()->getDynamicsWorld()->createRigidBody(rbInfo, shape);
            PhysicsRigidBodyClass::createInstanceFromPointer(vm, body);

            return 1;
        }
        return 0;
    }

    SQInteger PhysicsNamespace::addRigidBody(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            DynamicsWorld::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

            world->getPhysicsManager()->getDynamicsWorld()->addBody(body);
        }
        return 0;
    }

    SQInteger PhysicsNamespace::removeRigidBody(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            DynamicsWorld::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

            world->getPhysicsManager()->getDynamicsWorld()->removeBody(body);
        }
        return 0;
    }

    void PhysicsNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, getCubeShape, "getCubeShape", 4, ".nnn");
        _addFunction(vm, getSphereShape, "getSphereShape", 2, ".n");

        {
            //Create the dynamics namespace.
            sq_pushstring(vm, _SC("dynamics"), -1);
            sq_newtable(vm);

            ScriptUtils::addFunction(vm, createRigidBody, "createRigidBody", -2, ".xt");
            ScriptUtils::addFunction(vm, addRigidBody, "addBody", 2, ".x");
            ScriptUtils::addFunction(vm, removeRigidBody, "removeBody", 2, ".x");

            sq_newslot(vm, -3, false);
        }
    }
}
