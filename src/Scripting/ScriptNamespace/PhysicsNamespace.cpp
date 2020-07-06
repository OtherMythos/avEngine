#include "PhysicsNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"
#include "World/Physics/Worlds/CollisionWorld.h"
#include "World/Physics/PhysicsCollisionDataManager.h"

#include "btBulletDynamicsCommon.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "Scripting/ScriptNamespace/Classes/PhysicsClasses/PhysicsShapeClass.h"
#include "Scripting/ScriptNamespace/Classes/PhysicsClasses/PhysicsSenderClass.h"
#include "Scripting/ScriptNamespace/Classes/PhysicsClasses/PhysicsRigidBodyClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/SystemSetup/SystemSettings.h"

namespace AV {
    SQInteger PhysicsNamespace::getCubeShape(HSQUIRRELVM vm){
        SQFloat x, y, z;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);

        PhysicsTypes::ShapePtr shape = PhysicsShapeManager::getBoxShape(btVector3(x, y, z));
        PhysicsShapeClass::createInstanceFromPointer(vm, shape);

        return 1;
    }

    SQInteger PhysicsNamespace::getSphereShape(HSQUIRRELVM vm){
        SQFloat radius;
        sq_getfloat(vm, -1, &radius);

        PhysicsTypes::ShapePtr shape = PhysicsShapeManager::getSphereShape(radius);
        PhysicsShapeClass::createInstanceFromPointer(vm, shape);

        return 1;
    }

    SQInteger PhysicsNamespace::getCapsuleShape(HSQUIRRELVM vm){
        SQFloat radius, height;
        sq_getfloat(vm, -1, &height);
        sq_getfloat(vm, -2, &radius);

        PhysicsTypes::ShapePtr shape = PhysicsShapeManager::getCapsuleShape(radius, height);
        PhysicsShapeClass::createInstanceFromPointer(vm, shape);

        return 1;
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
        btRigidBody::btRigidBodyConstructionInfo rbInfo(1, 0, 0);
        PhysicsTypes::ShapePtr shape;

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

        PhysicsTypes::RigidBodyPtr body = PhysicsBodyConstructor::createRigidBody(rbInfo, shape);
        PhysicsRigidBodyClass::createInstanceFromPointer(vm, body);

        return 1;
    }

    SQInteger PhysicsNamespace::addRigidBody(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

            world->getPhysicsManager()->getDynamicsWorld()->addBody(body);
        }
        return 0;
    }

    SQInteger PhysicsNamespace::removeRigidBody(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

            world->getPhysicsManager()->getDynamicsWorld()->removeBody(body);
        }
        return 0;
    }

    void PhysicsNamespace::_iterateSenderConstructionTable(HSQUIRRELVM vm, SQInteger idx, SenderConstructionInfo* outInfo){
        //Reset the values
        sq_resetobject(&(outInfo->closure));
        outInfo->filePath = 0;
        outInfo->funcName = 0;
        outInfo->closureParams = 0;
        outInfo->userId = 0;

        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, idx))){
            const SQChar *k;
            sq_getstring(vm, -2, &k);

            SQObjectType t = sq_gettype(vm, -1);
            if(t == OT_INTEGER){
                SQInteger val;
                sq_getinteger(vm, -1, &val);

                if(strcmp(k, "id") == 0){
                    outInfo->userId = val;
                }
                else if(strcmp(k, "type") == 0){
                    outInfo->objType = (CollisionObjectTypeMask::CollisionObjectTypeMask)val;
                }
                else if(strcmp(k, "event") == 0){
                    outInfo->eventType = (CollisionObjectEventMask::CollisionObjectEventMask)val;
                }
            }else if(t == OT_STRING){
                const SQChar *foundString;
                sq_getstring(vm, -1, &foundString);
                if(strcmp(k, "path") == 0){
                    outInfo->filePath = foundString;
                }
                else if(strcmp(k, "func") == 0){
                    outInfo->funcName = foundString;
                }
            }
            else if(t == OT_CLOSURE){
                if(strcmp(k, "func") == 0){
                    sq_getstackobj(vm, -1, &(outInfo->closure));

                    SQInteger numParams, numFreeVariables;
                    sq_getclosureinfo(vm, -1, &numParams, &numFreeVariables);

                    if(numParams < 255){
                        outInfo->closureParams = uint8(numParams);
                    }
                }
            }

            sq_pop(vm,2); //pop the key and value
        }

        sq_pop(vm,1); //pops the null iterator
    }

    SQInteger PhysicsNamespace::_createCollisionObject(HSQUIRRELVM vm, bool isSender){
        SQInteger stackSize = sq_gettop(vm);

        PhysicsTypes::ShapePtr shape;
        Ogre::Vector3 origin(Ogre::Vector3::ZERO);
        SenderConstructionInfo info;
        _iterateSenderConstructionTable(vm, 2, &info);
        shape = PhysicsShapeClass::getPointerFromInstance(vm, 3);

        if(stackSize > 3){
            //table, shape and position.
            if(!ScriptGetterUtils::vector3ReadSlotOrVec(vm, &origin, 4)) return 0;
        }


        CollisionObjectType::CollisionObjectType objType = isSender ? CollisionObjectType::SENDER_SCRIPT : CollisionObjectType::RECEIVER;
        void* storedData = INVALID_DATA_ID;
        if(isSender){
            if(info.closureParams > 0){
                storedData = PhysicsCollisionDataManager::createCollisionSenderClosureFromData(info.closure, info.closureParams, info.userId);
                objType = CollisionObjectType::SENDER_CLOSURE;
            }else{
                if(info.filePath && info.funcName){
                    storedData = PhysicsCollisionDataManager::createCollisionSenderScriptFromData(info.filePath, info.funcName, info.userId);
                }
            }
        }
        CollisionPackedInt packedInt = CollisionWorldUtils::producePackedInt(objType, info.objType, info.eventType);

        PhysicsTypes::CollisionObjectPtr obj = PhysicsBodyConstructor::createCollisionObject(shape, packedInt, storedData, OGRE_TO_BULLET(origin));
        PhysicsSenderClass::createInstanceFromPointer(vm, obj, !isSender);

        return 1;
    }

    SQInteger PhysicsNamespace::createCollisionSender(HSQUIRRELVM vm){
        return _createCollisionObject(vm, true);
    }

    SQInteger PhysicsNamespace::createCollisionReceiver(HSQUIRRELVM vm){
        return _createCollisionObject(vm, false);
    }

    SQInteger PhysicsNamespace::addCollisionObject(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            PhysicsTypes::CollisionObjectPtr obj;
            bool success = PhysicsSenderClass::getPointerFromInstance(vm, -1, &obj, PhysicsSenderClass::EITHER);
            if(!success) return sq_throwerror(vm, "Invalid object passed");

            //TODO defaults to 0 for now.
            world->getPhysicsManager()->getCollisionWorld(0)->addObject(obj);
        }
        return 0;
    }

    SQInteger PhysicsNamespace::removeCollisionObject(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            PhysicsTypes::CollisionObjectPtr obj;
            bool success = PhysicsSenderClass::getPointerFromInstance(vm, -1, &obj, PhysicsSenderClass::EITHER);
            if(!success) return sq_throwerror(vm, "Invalid object passed");

            //TODO defaults to 0 for now.
            world->getPhysicsManager()->getCollisionWorld(0)->removeObject(obj);
        }
        return 0;
    }

    /**SQNamespace
    @name _physics
    @desc Functions to do things with physics.
    */
    void PhysicsNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name getCubeShape
        @desc Get a cube shape instance (bullet box shape).
        @returns An instance of a cube shape.
        @param1:x: Half extends x
        @param2:y: Half extends y
        @param3:z: Half extends x
        */
        ScriptUtils::addFunction(vm, getCubeShape, "getCubeShape", 4, ".nnn");
        /**SQFunction
        @name getSphereShape
        @desc Get a sphere shape instance.
        @returns An instance of a sphere shape
        @param1:radius: Sphere radius
        */
        ScriptUtils::addFunction(vm, getSphereShape, "getSphereShape", 2, ".n");
        /**SQFunction
        @name getCapsuleShape
        @desc Get a capsule shape instance.
        @returns An instance of a sphere shape
        */
        ScriptUtils::addFunction(vm, getCapsuleShape, "getCapsuleShape", 3, ".nn");

        {
            //Create the dynamics namespace.
            sq_pushstring(vm, _SC("dynamics"), -1);
            sq_newtable(vm);

            ScriptUtils::addFunction(vm, createRigidBody, "createRigidBody", -2, ".xt");
            ScriptUtils::addFunction(vm, addRigidBody, "addBody", 2, ".x");
            ScriptUtils::addFunction(vm, removeRigidBody, "removeBody", 2, ".x");

            sq_newslot(vm, -3, false);
        }

        {
            //Collision namespace.
            sq_pushstring(vm, _SC("collision"), -1);
            //sq_newtable(vm);

            int collisionWorlds = SystemSettings::getNumCollisionWorlds();
            //TODO in future, rather than having an array I could have a table which has the _get metamethod filled. Then I could do some error checking and provide helpful feedback to the user about why something was invalid.
            sq_newarray(vm, collisionWorlds);

            for(int i = 0; i < collisionWorlds; i++){
                sq_newtable(vm);

                ScriptUtils::addFunction(vm, createCollisionSender, "createSender", -3, ".txu|x");
                ScriptUtils::addFunction(vm, createCollisionReceiver, "createReceiver", -3, ".txu|x");
                ScriptUtils::addFunction(vm, addCollisionObject, "addObject", 2, ".u");
                ScriptUtils::addFunction(vm, removeCollisionObject, "removeObject", 2, ".u");

                //Insert means you can pre-allocate the size of the array and just insert into it. Append would start to push ontop of the array.
                sq_arrayinsert(vm, -2, i);
            }

            sq_newslot(vm, -3, false);
        }

    }

    void PhysicsNamespace::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_COLLISION_PLAYER", (SQInteger)CollisionObjectTypeMask::PLAYER);
        ScriptUtils::declareConstant(vm, "_COLLISION_ENEMY", (SQInteger)CollisionObjectTypeMask::ENEMY);
        ScriptUtils::declareConstant(vm, "_COLLISION_OBJECT", (SQInteger)CollisionObjectTypeMask::OBJECT);
        ScriptUtils::declareConstant(vm, "_COLLISION_USER_3", (SQInteger)CollisionObjectTypeMask::USER_3);
        ScriptUtils::declareConstant(vm, "_COLLISION_USER_4", (SQInteger)CollisionObjectTypeMask::USER_4);
        ScriptUtils::declareConstant(vm, "_COLLISION_USER_5", (SQInteger)CollisionObjectTypeMask::USER_5);
        ScriptUtils::declareConstant(vm, "_COLLISION_USER_6", (SQInteger)CollisionObjectTypeMask::USER_6);

        ScriptUtils::declareConstant(vm, "_COLLISION_LEAVE", (SQInteger)CollisionObjectEventMask::LEAVE);
        ScriptUtils::declareConstant(vm, "_COLLISION_ENTER", (SQInteger)CollisionObjectEventMask::ENTER);
        ScriptUtils::declareConstant(vm, "_COLLISION_INSIDE", (SQInteger)CollisionObjectEventMask::INSIDE);
    }
}
