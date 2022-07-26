#include "EntityUserData.h"

#include "World/WorldSingleton.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "World/Entity/EntityManager.h"

#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"

#include "World/Slot/ChunkRadiusLoader.h"
#include "World/Entity/Logic/FundamentalLogic.h"
#include "Scripting/ScriptObjectTypeTags.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "System/EngineFlags.h"

namespace AV{

    SQObject EntityUserData::eIdDelegateTable;

    SQInteger EntityUserData::setEntityPosition(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, -2, &entityId));

            SlotPosition pos;
            SCRIPT_CHECK_RESULT(SlotPositionClass::getSlotFromInstance(vm, -1, &pos));

            world->getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityUserData::getEntityPosition(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, -1, &entityId));

            SlotPosition pos = FundamentalLogic::getPosition(entityId);

            //Push a slotPositionClass instance to the stack.
            SlotPositionClass::createNewInstance(vm, pos);
        }
        return 1;
    }

    SQInteger EntityUserData::checkValid(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(!world){
            //This is working under the assumption that if there is no world there can't be a valid entity.
            sq_pushbool(vm, false);
            return 1;
        }

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, -1, &entityId));

            bool retVal = world->getEntityManager()->getEntityValid(entityId);
            sq_pushbool(vm, retVal);
        }
        return 1;
    }

    SQInteger EntityUserData::checkTrackable(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, -1, &entityId));

            SlotPosition pos = FundamentalLogic::getPosition(entityId);
            bool viableChunk = world->getChunkRadiusLoader()->chunkLoadedInCurrentMap(pos.chunkX(), pos.chunkY());

            sq_pushbool(vm, viableChunk);
        }
        return 1;
    }

    SQInteger EntityUserData::isTracked(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, -1, &entityId));

            SQBool tracked = FundamentalLogic::getTracked(entityId);

            sq_pushbool(vm, tracked);
        }
        return 1;
    }

    SQInteger EntityUserData::moveEntity(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, 1, &entityId));

            Ogre::Vector3 amount;
            SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &amount));

            SlotPosition pos = FundamentalLogic::getPosition(entityId);
            pos += amount;

            world->getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityUserData::moveTowards(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, 1, &entityId));

            SlotPosition destination;
            SCRIPT_CHECK_RESULT(SlotPositionClass::getSlotFromInstance(vm, 2, &destination));

            SQFloat amount = 0.0f;
            sq_getfloat(vm, 3, &amount);

            SlotPosition pos = FundamentalLogic::getPosition(entityId);
            pos.moveTowards(destination, amount);

            world->getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityUserData::getEntityId(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, 1, &entityId));

            sq_pushinteger(vm, static_cast<SQInteger>(entityId.id()));
        }
        return 1;
    }

    SQInteger EntityUserData::_entityCompare(HSQUIRRELVM vm){
        eId pf, ps;
        { SCRIPT_ASSERT_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &pf)); }
        { SCRIPT_ASSERT_RESULT(EntityUserData::readeIDFromUserData(vm, -2, &ps)); }

        if(pf == ps){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushbool(vm, false);
        }
        return 1;
    }

    void EntityUserData::eIDToUserData(HSQUIRRELVM vm, eId entity){
        eId* pointer = (eId*)sq_newuserdata(vm, sizeof(eId));
        *pointer = entity;

        sq_pushobject(vm, eIdDelegateTable);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, EntityUserDataTypeTag);
    }

    UserDataGetResult EntityUserData::_readeIDFromUserDataPointer(HSQUIRRELVM vm, int stackIndex, eId** outEID){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackIndex, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != EntityUserDataTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        eId* p = static_cast<eId*>(pointer);
        *outEID = p;

        return USER_DATA_GET_SUCCESS;
    }

    UserDataGetResult EntityUserData::readeIDFromUserData(HSQUIRRELVM vm, int stackIndex, eId* outEID){
        eId* p = 0;
        UserDataGetResult result = _readeIDFromUserDataPointer(vm, stackIndex, &p);
        *outEID = *p;

        return result;
    }

    void EntityUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 9);

        ScriptUtils::addFunction(vm, setEntityPosition, "setPosition", 2, ".u");
        ScriptUtils::addFunction(vm, getEntityPosition, "getPosition");
        ScriptUtils::addFunction(vm, moveEntity, "move", -2, ".u|nnn");
        ScriptUtils::addFunction(vm, _entityCompare, "_cmp");
        ScriptUtils::addFunction(vm, checkValid, "valid");
        ScriptUtils::addFunction(vm, checkTrackable, "trackable");
        ScriptUtils::addFunction(vm, isTracked, "tracked");
        ScriptUtils::addFunction(vm, moveTowards, "moveTowards", 3, ".un");
        ScriptUtils::addFunction(vm, getEntityId, "getId");

        sq_resetobject(&eIdDelegateTable);
        sq_getstackobj(vm, -1, &eIdDelegateTable);
        sq_addref(vm, &eIdDelegateTable);
        sq_pop(vm, 1);
    }
}
