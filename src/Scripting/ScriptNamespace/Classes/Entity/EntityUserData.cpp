#include "EntityUserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "System/BaseSingleton.h"

#include "Entity/EntityManager.h"

#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"

#include "Entity/Logic/FundamentalLogic.h"
#include "Scripting/ScriptObjectTypeTags.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "System/EngineFlags.h"

namespace AV{

    SQObject EntityUserData::eIdDelegateTable;

    SQInteger EntityUserData::setEntityPosition(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, -2, &entityId));

            Ogre::Vector3 pos;
            SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, -1, &pos));

            BaseSingleton::getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityUserData::getEntityPosition(HSQUIRRELVM vm){

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, -1, &entityId));

            Ogre::Vector3 pos = FundamentalLogic::getPosition(entityId);

            //Push a slotPositionClass instance to the stack.
            Vector3UserData::vector3ToUserData(vm, pos);
        }
        return 1;
    }

    SQInteger EntityUserData::checkValid(HSQUIRRELVM vm){
        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, -1, &entityId));

            bool retVal = BaseSingleton::getEntityManager()->getEntityValid(entityId);
            sq_pushbool(vm, retVal);
        }
        return 1;
    }

    SQInteger EntityUserData::moveEntity(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, 1, &entityId));

            Ogre::Vector3 amount;
            SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &amount));

            Ogre::Vector3 pos = FundamentalLogic::getPosition(entityId);
            pos += amount;

            BaseSingleton::getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityUserData::moveTowards(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()

        {
            eId entityId;
            SCRIPT_ASSERT_RESULT(readeIDFromUserData(vm, 1, &entityId));

            Ogre::Vector3 destination;
            SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 2, &destination));

            SQFloat amount = 0.0f;
            sq_getfloat(vm, 3, &amount);

            Ogre::Vector3 pos = FundamentalLogic::getPosition(entityId);
            const Ogre::Vector3 delta(destination - pos);
            const Ogre::Real magnitude = delta.length();
            if(magnitude <= amount || magnitude == 0.0f) pos = destination;
            else pos += (delta / magnitude * amount);

            BaseSingleton::getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityUserData::getEntityId(HSQUIRRELVM vm){

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
        ScriptUtils::addFunction(vm, moveTowards, "moveTowards", 3, ".un");
        ScriptUtils::addFunction(vm, getEntityId, "getId");

        sq_resetobject(&eIdDelegateTable);
        sq_getstackobj(vm, -1, &eIdDelegateTable);
        sq_addref(vm, &eIdDelegateTable);
        sq_pop(vm, 1);
    }
}
