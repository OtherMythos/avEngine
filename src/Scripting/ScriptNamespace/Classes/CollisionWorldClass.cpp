#include "CollisionWorldClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "System/Util/Collision/CollisionWorldObject.h"
#include "System/Util/Collision/CollisionWorldBruteForce.h"
#include "System/Util/Collision/CollisionWorldOctree.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/Classes/Vector2UserData.h"

namespace AV{
    SQObject CollisionWorldClass::collisionWorldDelegateTableObject;

    void CollisionWorldClass::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, processCollision, "processCollision");
        ScriptUtils::addFunction(vm, addCollisionPoint, "addCollisionPoint", -4, ".nnni");
        ScriptUtils::addFunction(vm, addCollisionRectangle, "addCollisionRectangle", -4, ".nnnni");
        ScriptUtils::addFunction(vm, addCollisionRotatedRectangle, "addCollisionRotatedRectangle", -5, ".nnnnni");
        ScriptUtils::addFunction(vm, checkCollisionPoint, "checkCollisionPoint", -3, ".nnnii");
        ScriptUtils::addFunction(vm, removeCollisionPoint, "removeCollisionPoint", 2, ".i");
        ScriptUtils::addFunction(vm, getNumCollisions, "getNumCollisions");
        ScriptUtils::addFunction(vm, getCollisionPairForIdx, "getCollisionPairForIdx", 2, ".i");
        ScriptUtils::addFunction(vm, setPositionForPoint, "setPositionForPoint", 4, ".inn");
        ScriptUtils::addFunction(vm, getPositionForPoint, "getPositionForPoint", 2, ".i");
        ScriptUtils::addFunction(vm, setUserValue, "setUserValue", 3, ".ii");
        ScriptUtils::addFunction(vm, getUserValue, "getUserValue", 2, ".i");

        sq_resetobject(&collisionWorldDelegateTableObject);
        sq_getstackobj(vm, -1, &collisionWorldDelegateTableObject);
        sq_addref(vm, &collisionWorldDelegateTableObject);
        sq_pop(vm, 1);

        //Create the creation functions.
        sq_pushroottable(vm);

        {
            ScriptUtils::addFunction(vm, createCollisionWorld, "CollisionWorld", -2, ".ii");
        }

    }

    void CollisionWorldClass::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_COLLISION_WORLD_BRUTE_FORCE", (SQInteger)CollisionWorldType::WorldBruteForce);
        ScriptUtils::declareConstant(vm, "_COLLISION_WORLD_OCTREE", (SQInteger)CollisionWorldType::WorldOctree);

        ScriptUtils::declareConstant(vm, "_COLLISION_WORLD_ENTRY_EITHER", (SQInteger)CollisionEntryType::either);
        ScriptUtils::declareConstant(vm, "_COLLISION_WORLD_ENTRY_SENDER", (SQInteger)CollisionEntryType::sender);
        ScriptUtils::declareConstant(vm, "_COLLISION_WORLD_ENTRY_RECEIVER", (SQInteger)CollisionEntryType::receiver);
    }

    SQInteger CollisionWorldClass::createCollisionWorld(HSQUIRRELVM vm){
        SQInteger worldType;
        sq_getinteger(vm, 2, &worldType);

        SQInteger worldId = -1;
        if(sq_gettop(vm) >= 3){
            sq_getinteger(vm, 3, &worldId);
        }

        CollisionWorldObject* outWorld;
        if(worldType == CollisionWorldType::WorldBruteForce){
            CollisionWorldBruteForce* bruteForce = new CollisionWorldBruteForce(static_cast<int>(worldId));
            outWorld = dynamic_cast<CollisionWorldObject*>(bruteForce);
        }else if(worldType == CollisionWorldType::WorldOctree){
            CollisionWorldOctree* octree = new CollisionWorldOctree(static_cast<int>(worldId));
            outWorld = dynamic_cast<CollisionWorldObject*>(octree);
        }else{
            return sq_throwerror(vm, "Unknown collision world type requested.");
        }
        assert(outWorld);

        collisionWorldToUserData(vm, outWorld);

        return 1;
    }

    SQInteger CollisionWorldClass::checkCollisionPoint(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        SQFloat x, y, radius;

        sq_getfloat(vm, 2, &x);
        sq_getfloat(vm, 3, &y);
        sq_getfloat(vm, 4, &radius);

        uint8 mask = 0xFF;
        if(sq_gettop(vm) >= 5){
            SQInteger outMask;
            sq_getinteger(vm, 5, &outMask);
            mask = static_cast<uint8>(outMask);
        }

        CollisionEntryId ignorePointId = COLLISION_ENTRY_ID_INVALID;
        if(sq_gettop(vm) >= 6){
            SQInteger outIgnoreId;
            sq_getinteger(vm, 6, &outIgnoreId);
            ignorePointId = static_cast<CollisionEntryId>(outIgnoreId);
        }

        bool result = outWorld->checkCollisionPoint(x, y, radius, mask, ignorePointId);

        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger CollisionWorldClass::addCollisionPoint(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        SQFloat x, y, radius;

        sq_getfloat(vm, 2, &x);
        sq_getfloat(vm, 3, &y);
        sq_getfloat(vm, 4, &radius);

        uint8 targetMask = 0xFF;
        if(sq_gettop(vm) >= 5){
            SQInteger outMask;
            sq_getinteger(vm, 5, &outMask);
            targetMask = static_cast<uint8>(outMask);
        }
        CollisionEntryType targetEntryType = CollisionEntryType::either;
        if(sq_gettop(vm) >= 6){
            SQInteger outType;
            sq_getinteger(vm, 6, &outType);
            targetEntryType = static_cast<CollisionEntryType>(outType);
        }

        CollisionEntryId entryId = outWorld->addCollisionPoint(x, y, radius, targetMask, targetEntryType);

        sq_pushinteger(vm, static_cast<SQInteger>(entryId));

        return 1;
    }

    SQInteger CollisionWorldClass::addCollisionRectangle(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        SQFloat x, y, width, height;

        sq_getfloat(vm, 2, &x);
        sq_getfloat(vm, 3, &y);
        sq_getfloat(vm, 4, &width);
        sq_getfloat(vm, 5, &height);

        uint8 targetMask = 0xFF;
        if(sq_gettop(vm) >= 6){
            SQInteger outMask;
            sq_getinteger(vm, 6, &outMask);
            targetMask = static_cast<uint8>(outMask);
        }
        CollisionEntryType targetEntryType = CollisionEntryType::either;
        if(sq_gettop(vm) >= 7){
            SQInteger outType;
            sq_getinteger(vm, 7, &outType);
            targetEntryType = static_cast<CollisionEntryType>(outType);
        }

        CollisionEntryId entryId = outWorld->addCollisionRectangle(x, y, width, height, targetMask, targetEntryType);

        sq_pushinteger(vm, static_cast<SQInteger>(entryId));

        return 1;
    }

    SQInteger CollisionWorldClass::addCollisionRotatedRectangle(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        SQFloat x, y, width, height, rotation;

        sq_getfloat(vm, 2, &x);
        sq_getfloat(vm, 3, &y);
        sq_getfloat(vm, 4, &width);
        sq_getfloat(vm, 5, &height);
        sq_getfloat(vm, 6, &rotation);

        uint8 targetMask = 0xFF;
        if(sq_gettop(vm) >= 7){
            SQInteger outMask;
            sq_getinteger(vm, 7, &outMask);
            targetMask = static_cast<uint8>(outMask);
        }
        CollisionEntryType targetEntryType = CollisionEntryType::either;
        if(sq_gettop(vm) >= 8){
            SQInteger outType;
            sq_getinteger(vm, 8, &outType);
            targetEntryType = static_cast<CollisionEntryType>(outType);
        }

        CollisionEntryId entryId = outWorld->addCollisionRotatedRectangle(x, y, width, height, rotation, targetMask, targetEntryType);

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

        outWorld->removeCollisionEntry(static_cast<CollisionEntryId>(entryId));

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

    SQInteger CollisionWorldClass::getPositionForPoint(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        SQInteger pairIdx;
        sq_getinteger(vm, 2, &pairIdx);

        float posX, posY;
        bool result = outWorld->getPositionForPoint(static_cast<CollisionEntryId>(pairIdx), &posX, &posY);
        if(!result){
            return sq_throwerror(vm, "Invalid collision entry id");
        }

        Vector2UserData::vector2ToUserData(vm, Ogre::Vector2(posX, posY));

        return 1;
    }

    SQInteger CollisionWorldClass::setUserValue(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        SQInteger entryId;
        SQInteger userValue;
        sq_getinteger(vm, 2, &entryId);
        sq_getinteger(vm, 3, &userValue);

        outWorld->setUserValue(static_cast<CollisionEntryId>(entryId), static_cast<uint64>(userValue));

        return 0;
    }

    SQInteger CollisionWorldClass::getUserValue(HSQUIRRELVM vm){
        CollisionWorldObject* outWorld = 0;
        SCRIPT_ASSERT_RESULT(readCollisionWorldFromUserData(vm, 1, &outWorld));

        SQInteger entryId;
        sq_getinteger(vm, 2, &entryId);

        uint64 val = outWorld->getUserValue(static_cast<CollisionEntryId>(entryId));

        sq_pushinteger(vm, static_cast<SQInteger>(val));

        return 1;
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
        sq_setreleasehook(vm, -1, collisionWorldReleaseHook);
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
