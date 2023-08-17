#include "CollisionWorldClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "System/Util/Collision/CollisionWorldObject.h"
#include "System/Util/Collision/CollisionWorldBruteForce.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{
    SQObject CollisionWorldClass::collisionWorldDelegateTableObject;

    void CollisionWorldClass::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, processCollision, "processCollision");
        ScriptUtils::addFunction(vm, addCollisionPoint, "addCollisionPoint", 4, ".nnn");
        ScriptUtils::addFunction(vm, removeCollisionPoint, "removeCollisionPoint", 2, ".i");
        ScriptUtils::addFunction(vm, getNumCollisions, "getNumCollisions");
        ScriptUtils::addFunction(vm, getCollisionPairForIdx, "getCollisionPairForIdx", 2, ".i");
        ScriptUtils::addFunction(vm, setPositionForPoint, "setPositionForPoint", 4, ".inn");

        sq_resetobject(&collisionWorldDelegateTableObject);
        sq_getstackobj(vm, -1, &collisionWorldDelegateTableObject);
        sq_addref(vm, &collisionWorldDelegateTableObject);
        sq_pop(vm, 1);

        //Create the creation functions.
        sq_pushroottable(vm);

        {
            ScriptUtils::addFunction(vm, createCollisionWorld, "CollisionWorld", 2, ".i");
        }

    }

    void CollisionWorldClass::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_COLLISION_WORLD_BRUTE_FORCE", (SQInteger)CollisionWorldType::WorldBruteForce);
    }

    SQInteger CollisionWorldClass::createCollisionWorld(HSQUIRRELVM vm){
        SQInteger worldType;
        sq_getinteger(vm, -1, &worldType);

        CollisionWorldObject* outWorld;
        if(worldType == CollisionWorldType::WorldBruteForce){
            CollisionWorldBruteForce* bruteForce = new CollisionWorldBruteForce();
            outWorld = dynamic_cast<CollisionWorldObject*>(bruteForce);
        }else{
            return sq_throwerror(vm, "Unknown collision world type requested.");
        }
        assert(outWorld);

        collisionWorldToUserData(vm, outWorld);

        return 1;
    }

    SQInteger CollisionWorldClass::addCollisionPoint(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        SQFloat x, y, radius;

        sq_getfloat(vm, 2, &x);
        sq_getfloat(vm, 3, &y);
        sq_getfloat(vm, 4, &radius);

        CollisionEntryId entryId = outWorld->addCollisionPoint(x, y, radius);

        sq_pushinteger(vm, static_cast<SQInteger>(entryId));

        return 1;
    }

    SQInteger CollisionWorldClass::removeCollisionPoint(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        SQInteger entryId;

        sq_getinteger(vm, 2, &entryId);

        if(entryId < 0){
            return sq_throwerror(vm, "Provided value must be positive.");
        }

        outWorld->removeCollisionPoint(static_cast<CollisionEntryId>(entryId));

        return 0;
    }

    SQInteger CollisionWorldClass::processCollision(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        outWorld->processCollision();

        return 0;
    }

    SQInteger CollisionWorldClass::getNumCollisions(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        int collisions = outWorld->getNumCollisions();
        sq_pushinteger(vm, collisions);

        return 1;
    }

    SQInteger CollisionWorldClass::getCollisionPairForIdx(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        SQInteger pairIdx;
        sq_getinteger(vm, 2, &pairIdx);

        CollisionPackedResult result = outWorld->getCollisionPairForIdx(static_cast<int>(pairIdx));
        if(result == COLLISION_PACKED_RESULT_INVALID){
            return sq_throwerror(vm, "Invalid collision id");
        }

        sq_pushinteger(vm, result);

        return 1;
    }

    SQInteger CollisionWorldClass::setPositionForPoint(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        SQInteger pairIdx;
        sq_getinteger(vm, 2, &pairIdx);

        SQFloat posX;
        SQFloat posY;
        sq_getfloat(vm, 3, &posX);
        sq_getfloat(vm, 4, &posY);

        outWorld->setPositionForPoint(static_cast<CollisionEntryId>(pairIdx), posX, posY);

        return 0;
    }

    SQInteger CollisionWorldClass::collisionWorldReleaseHook(SQUserPointer p, SQInteger size){
        CollisionWorldObject** outObj = static_cast<CollisionWorldObject**>(p);

        delete *outObj;

        return 0;
    }

    void CollisionWorldClass::collisionWorldToUserData(HSQUIRRELVM vm, CollisionWorldObject* world){
        CollisionWorldObject** pointer = (CollisionWorldObject**)sq_newuserdata(vm, sizeof(CollisionWorldObject*));
        *pointer = world;

        sq_pushobject(vm, collisionWorldDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, CollisionWorldTypeTag);
        sq_setreleasehook(vm, 1, collisionWorldReleaseHook);
    }

    UserDataGetResult CollisionWorldClass::readCollisionWorldFromUserData(HSQUIRRELVM vm, SQInteger stackInx, CollisionWorldObject** outWorld){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != CollisionWorldTypeTag){
            *outWorld = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outWorld = *((CollisionWorldObject**)pointer);

        return USER_DATA_GET_SUCCESS;
    }
}
