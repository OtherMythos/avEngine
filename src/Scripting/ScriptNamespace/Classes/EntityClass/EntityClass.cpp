#include "EntityClass.h"

#include "World/WorldSingleton.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "World/Entity/EntityManager.h"

#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"

#include "World/Slot/ChunkRadiusLoader.h"
#include "World/Entity/Logic/FundamentalLogic.h"
#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    SQObject EntityClass::classObject;

    DataPacker<uint64_t> EntityClass::eIdData;

    SQInteger EntityClass::setEntityPosition(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(getEID(vm, -2, &entityId));

            SlotPosition pos;
            SCRIPT_CHECK_RESULT(SlotPositionClass::getSlotFromInstance(vm, -1, &pos));

            world->getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityClass::getEntityPosition(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(getEID(vm, -1, &entityId));

            SlotPosition pos = FundamentalLogic::getPosition(entityId);

            //Push a slotPositionClass instance to the stack.
            SlotPositionClass::createNewInstance(vm, pos);
        }
        return 1;
    }

    SQInteger EntityClass::checkValid(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(!world){
            //This is working under the assumption that if there is no world there can't be a valid entity.
            sq_pushbool(vm, false);
            return 1;
        }

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(getEID(vm, -1, &entityId));

            bool retVal = world->getEntityManager()->getEntityValid(entityId);
            sq_pushbool(vm, retVal);
        }
        return 1;
    }

    void EntityClass::invalidateEntityInstance(HSQUIRRELVM vm){
        SQUserPointer p = 0;
        sq_getinstanceup(vm, -1, &p, EntityClassTypeTag);

        eIdData.setEntry(p, 0);
    }

    SQInteger EntityClass::checkTrackable(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(getEID(vm, -1, &entityId));

            SlotPosition pos = FundamentalLogic::getPosition(entityId);
            bool viableChunk = world->getChunkRadiusLoader()->chunkLoadedInCurrentMap(pos.chunkX(), pos.chunkY());

            sq_pushbool(vm, viableChunk);
        }
        return 1;
    }

    UserDataGetResult EntityClass::getEID(HSQUIRRELVM vm, int stackIndex, eId* outEID){
        SQUserPointer p, typeTag;
        //TODO this user data stuff doesn't make much sense for a class. I should consider changing to something else.
        if(SQ_FAILED(sq_getinstanceup(vm, stackIndex, &p, EntityClassTypeTag))) return USER_DATA_GET_INCORRECT_TYPE;

        eId e(eIdData.getEntry(p));
        *outEID = e;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger EntityClass::isTracked(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(getEID(vm, -1, &entityId));

            SQBool tracked = FundamentalLogic::getTracked(entityId);

            sq_pushbool(vm, tracked);
        }
        return 1;
    }

    SQInteger EntityClass::moveEntity(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(getEID(vm, 1, &entityId));

            Ogre::Vector3 amount;
            SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &amount));

            SlotPosition pos = FundamentalLogic::getPosition(entityId);
            pos += amount;

            world->getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityClass::_entityCompare(HSQUIRRELVM vm){
        SQUserPointer pf, ps;
        sq_getinstanceup(vm, -1, &pf, EntityClassTypeTag);
        sq_getinstanceup(vm, -2, &ps, EntityClassTypeTag);

        uint64_t first = eIdData.getEntry((void*)pf);
        uint64_t second = eIdData.getEntry((void*)ps);

        if(first == second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushbool(vm, false);
        }
        return 1;
    }

    void EntityClass::_entityClassFromEID(HSQUIRRELVM vm, eId entity){
        sq_pushobject(vm, classObject);

        sq_createinstance(vm, -1);

        void* id = eIdData.storeEntry(entity.id());
        sq_setinstanceup(vm, -1, (SQUserPointer*)id);

        sq_setreleasehook(vm, -1, EIDReleaseHook);

        //Remove the class from the stack.
        //I'm not sure I like this approach, given how it's not really a stack this way.
        //TODO investigate transfering this to be a user data.
        sq_remove(vm, -2);
    }

    SQInteger EntityClass::EIDReleaseHook(SQUserPointer p, SQInteger size){
        eIdData.removeEntry((void*)p);

        return 0;
    }

    void EntityClass::setupClass(HSQUIRRELVM vm){
        sq_newclass(vm, 0);

        ScriptUtils::addFunction(vm, setEntityPosition, "setPosition", 2, ".u");
        ScriptUtils::addFunction(vm, getEntityPosition, "getPosition");
        ScriptUtils::addFunction(vm, moveEntity, "move", -2, ".u|nnn");
        ScriptUtils::addFunction(vm, _entityCompare, "_cmp");
        ScriptUtils::addFunction(vm, checkValid, "valid");
        ScriptUtils::addFunction(vm, checkTrackable, "trackable");
        ScriptUtils::addFunction(vm, isTracked, "tracked");

        sq_settypetag(vm, -1, EntityClassTypeTag);
        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }
}
