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
#include "Scripting/ScriptNamespace/Classes/PhysicsClasses/PhysicsObjectUserData.h"
#include "Scripting/ScriptNamespace/Classes/PhysicsClasses/PhysicsRigidBodyClass.h"

#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/SystemSetup/SystemSettings.h"

SQObject collisionWorldTables[4];

namespace AV {

    const char* _getCollisionWorldFailureReason(CollisionWorld::CollisionFunctionStatus result){
        switch(result){
            case CollisionWorld::ALREADY_IN_WORLD: return "Object already in world.";
            case CollisionWorld::NOT_IN_WORLD: return "Object not in world.";
            case CollisionWorld::NO_WORLD: return "Collision world does not exist.";
            case CollisionWorld::WRONG_WORLD: return "Object does not belong to this word.";
            default: assert(false);
        }
    }

    SQInteger PhysicsNamespace::getCubeShape(HSQUIRRELVM vm){
        CHECK_PHYSICS();
        SQFloat x, y, z;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);

        PhysicsTypes::ShapePtr shape = PhysicsShapeManager::getBoxShape(btVector3(x, y, z));
        PhysicsShapeClass::createInstanceFromPointer(vm, shape);

        return 1;
    }

    SQInteger PhysicsNamespace::getSphereShape(HSQUIRRELVM vm){
        CHECK_PHYSICS();
        SQFloat radius;
        sq_getfloat(vm, -1, &radius);

        PhysicsTypes::ShapePtr shape = PhysicsShapeManager::getSphereShape(radius);
        PhysicsShapeClass::createInstanceFromPointer(vm, shape);

        return 1;
    }

    SQInteger PhysicsNamespace::getCapsuleShape(HSQUIRRELVM vm){
        CHECK_PHYSICS();
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
                SQInteger arraySize = sq_getsize(vm, -1);
                if(arraySize == 3 && key == "origin"){
                    SQFloat vals[3];
                    ScriptUtils::getFloatArray<3>(vm, vals);

                    info.m_startWorldTransform.setOrigin(btVector3(vals[0], vals[1], vals[2]));
                }
                else if(arraySize == 4 && key == "rotation"){
                    SQFloat vals[4];
                    ScriptUtils::getFloatArray<4>(vm, vals);

                    info.m_startWorldTransform.setRotation(btQuaternion(vals[0], vals[1], vals[2], vals[3]));
                }
            }else if(t == OT_USERDATA){
                if(key == "origin"){
                    Ogre::Vector3 vec;
                    UserDataGetResult result = Vector3UserData::readVector3FromUserData(vm, -1, &vec);
                    if(result != USER_DATA_GET_SUCCESS) continue;

                    info.m_startWorldTransform.setOrigin(OGRE_TO_BULLET(vec));
                }
                else if(key == "rotation"){
                    Ogre::Quaternion quat;
                    UserDataGetResult result = QuaternionUserData::readQuaternionFromUserData(vm, -1, &quat);
                    if(result != USER_DATA_GET_SUCCESS) continue;

                    info.m_startWorldTransform.setRotation(OGRE_TO_BULLET_QUAT(quat));
                }
            }

            sq_pop(vm,2); //pop the key and value
        }

        sq_pop(vm,1); //pops the null iterator
    }

    SQInteger PhysicsNamespace::createRigidBody(HSQUIRRELVM vm){
        CHECK_DYNAMIC_PHYSICS();
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
        CHECK_DYNAMIC_PHYSICS();
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

            world->getPhysicsManager()->getDynamicsWorld()->addBody(body);
        }
        return 0;
    }

    SQInteger PhysicsNamespace::removeRigidBody(HSQUIRRELVM vm){
        CHECK_DYNAMIC_PHYSICS();
        SCRIPT_CHECK_WORLD();

        {
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
        outInfo->objType = static_cast<CollisionObjectTypeMask::CollisionObjectTypeMask>(0);
        outInfo->eventType = static_cast<CollisionObjectEventMask::CollisionObjectEventMask>(0);

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

    SQInteger PhysicsNamespace::_createCollisionObject(HSQUIRRELVM vm, bool isSender, uint8 collisionWorldId){
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

        CollisionPackedInt packedInt = CollisionWorldUtils::producePackedInt(objType, collisionWorldId, info.objType, info.eventType);

        PhysicsTypes::CollisionObjectPtr obj = PhysicsBodyConstructor::createCollisionObject(shape, packedInt, storedData, OGRE_TO_BULLET(origin));
        PhysicsObjectUserData::collisionObjectFromPointer(vm, obj, !isSender);

        return 1;
    }

    template <uint8 A>
    SQInteger PhysicsNamespace::createCollisionSender(HSQUIRRELVM vm){
        return _createCollisionObject(vm, true, A);
    }

    template <uint8 A>
    SQInteger PhysicsNamespace::createCollisionReceiver(HSQUIRRELVM vm){
        return _createCollisionObject(vm, false, A);
    }

    template <uint8 A>
    SQInteger PhysicsNamespace::addCollisionObject(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::CollisionObjectPtr obj;
            SCRIPT_CHECK_RESULT(PhysicsObjectUserData::getPointerFromUserData(vm, -1, &obj, PhysicsObjectUserData::EITHER));

            CollisionWorld::CollisionFunctionStatus result = world->getPhysicsManager()->getCollisionWorld(A)->addObject(obj);
            if(result > 0) return sq_throwerror(vm, _getCollisionWorldFailureReason(result));
        }
        return 0;
    }

    template <uint8 A>
    SQInteger PhysicsNamespace::removeCollisionObject(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            PhysicsTypes::CollisionObjectPtr obj;
            SCRIPT_CHECK_RESULT(PhysicsObjectUserData::getPointerFromUserData(vm, -1, &obj, PhysicsObjectUserData::EITHER));

            CollisionWorld::CollisionFunctionStatus result = world->getPhysicsManager()->getCollisionWorld(A)->removeObject(obj);
            if(result > 0) return sq_throwerror(vm, _getCollisionWorldFailureReason(result));
        }
        return 0;
    }

    SQInteger PhysicsNamespace::collisionWordGetMetamethod(HSQUIRRELVM vm){
        // const SQChar *key;
        // sq_getstring(vm, -1, &key);
        SQObjectType t = sq_gettype(vm, -1);
        if(t != OT_INTEGER) return sq_throwerror(vm, "An integer should be passed to reference a collision world.");

        SQInteger i;
        sq_getinteger(vm, -1, &i);

        if(i < 0 || i >= SystemSettings::getNumCollisionWorlds()) return sq_throwerror(vm, "Invalid collision world id.");

        sq_pushobject(vm, collisionWorldTables[i]);

        return 1;
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
            //Create an empty table, and assign another to it with the delegate table, containing the getter metamethod.
            sq_newtable(vm);

            {
                //Create the delegate table
                sq_newtableex(vm, 1);
                ScriptUtils::addFunction(vm, collisionWordGetMetamethod, "_get");
                assert(SQ_SUCCEEDED(sq_setdelegate(vm, -2)));
            }

            sq_newslot(vm, -3, false);


            /**
            Create the functions for each collision world.

            Here I'm using template functions to create the appropriate functions.
            A separate function is created for each with an id assigned to it, representing which collision world it operates on.
            */

            #define COLLISION_FUNCTIONS(nn) \
                createCollisionSender<nn>, \
                createCollisionReceiver<nn>, \
                addCollisionObject<nn>, \
                removeCollisionObject<nn>

            SQFUNCTION functions0[] = { COLLISION_FUNCTIONS(0) };
            SQFUNCTION functions1[] = { COLLISION_FUNCTIONS(1) };
            SQFUNCTION functions2[] = { COLLISION_FUNCTIONS(2) };
            SQFUNCTION functions3[] = { COLLISION_FUNCTIONS(3) };
            static_assert(MAX_COLLISION_WORLDS == 4, "Update the above code if changing the number of collision worlds.");


            SQFUNCTION* functions[] = { &(functions0[0]), &(functions1[0]), &(functions2[0]), &(functions3[0]) };
            //Create each collision world object for the array.
            //These are later returned as part of the metamethod.
            for(int i = 0; i < collisionWorlds; i++){
                sq_newtable(vm);


                ScriptUtils::addFunction(vm, (*(functions[i]+0)), "createSender", -3, ".txu|x");
                ScriptUtils::addFunction(vm, (*(functions[i]+1)), "createReceiver", -3, ".txu|x");
                ScriptUtils::addFunction(vm, (*(functions[i]+2)), "addObject", 2, ".u");
                ScriptUtils::addFunction(vm, (*(functions[i]+3)), "removeObject", 2, ".u");

                sq_resetobject( &(collisionWorldTables[i]) );
                sq_getstackobj(vm, -1,  &(collisionWorldTables[i]) );
                sq_addref(vm,  &(collisionWorldTables[i]) );
                sq_pop(vm, 1);
            }

            #undef COLLISION_FUNCTIONS
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
