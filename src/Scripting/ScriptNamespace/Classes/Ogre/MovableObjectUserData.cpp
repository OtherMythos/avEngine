#include "MovableObjectUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"

#include "DatablockUserData.h"
#include "OgreMovableObject.h"
#include "OgreItem.h"
#include "OgreLight.h"

namespace AV{

    SQObject MovableObjectUserData::itemDelegateTableObject;
    SQObject MovableObjectUserData::lightDelegateTableObject;

    bool _typeTagMovableObject(void* tag){
        return tag >= (void*)71 && tag < (void*)80;
    }

    void MovableObjectUserData::movableObjectToUserData(HSQUIRRELVM vm, Ogre::MovableObject* object, MovableObjectType type){
        Ogre::MovableObject** pointer = (Ogre::MovableObject**)sq_newuserdata(vm, sizeof(Ogre::MovableObject*));
        *pointer = object;

        void* targetTypeTag = 0;
        SQObject* targetTable = 0;
        switch(type){
            case MovableObjectType::Item:
                targetTypeTag = MovableObjectItemTypeTag;
                targetTable = &itemDelegateTableObject;
                break;
            case MovableObjectType::Light:
                targetTypeTag = MovableObjectLightTypeTag;
                targetTable = &lightDelegateTableObject;
                break;
        }
        //Write a thing to determine the delegate table. Create the other delegate table in the setup bit.
        sq_pushobject(vm, *targetTable);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, targetTypeTag);
    }

    UserDataGetResult MovableObjectUserData::readMovableObjectFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::MovableObject** outObject, MovableObjectType expectedType){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(expectedType == MovableObjectType::Any){
            if(!_typeTagMovableObject(typeTag)){
                *outObject = 0;
                return USER_DATA_GET_TYPE_MISMATCH;
            }
        }else{
            static void* const targetTags[] = {0, MovableObjectItemTypeTag, MovableObjectLightTypeTag};
            if(typeTag != targetTags[(size_t)expectedType]){
                *outObject = 0;
                return USER_DATA_GET_TYPE_MISMATCH;
            }
        }

        Ogre::MovableObject** p = (Ogre::MovableObject**)pointer;
        *outObject = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger MovableObjectUserData::setDatablock(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_CHECK_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Item));

        SQObjectType t = sq_gettype(vm, 2);
        Ogre::Item* targetItem = static_cast<Ogre::Item*>(outObject);
        if(t == OT_USERDATA){
            Ogre::HlmsDatablock* db = 0;
            bool result = DatablockUserData::getPtrFromUserData(vm, 2, &db);
            if(!result) return 0; //TODO update this to use the new error approach.

            targetItem->setDatablock(db);
        }else if(t == OT_STRING){
            const SQChar *dbPath;
            sq_getstring(vm, 2, &dbPath);

            targetItem->setDatablock(dbPath);
        }else{
            assert(false);
        }

        return 0;
    }

    SQInteger MovableObjectUserData::setLightType(HSQUIRRELVM vm){
        SQInteger idx = 0;
        sq_getinteger(vm, 2, &idx);
        if(idx < 0 || idx >= Ogre::Light::NUM_LIGHT_TYPES) return sq_throwerror(vm, "Invalid light type id");

        Ogre::MovableObject* outObj = 0;
        SCRIPT_CHECK_RESULT(readMovableObjectFromUserData(vm, 1, &outObj, MovableObjectType::Light));

        Ogre::Light* lightObj = static_cast<Ogre::Light*>(outObj);

        lightObj->setType( (Ogre::Light::LightTypes)idx);

        return 0;
    }

    SQInteger MovableObjectUserData::setLightPowerScale(HSQUIRRELVM vm){
        SQFloat power = 0;
        sq_getfloat(vm, 2, &power);

        Ogre::MovableObject* outObj = 0;
        SCRIPT_CHECK_RESULT(readMovableObjectFromUserData(vm, 1, &outObj, MovableObjectType::Light));

        Ogre::Light* lightObj = static_cast<Ogre::Light*>(outObj);

        lightObj->setPowerScale(power);

        return 0;
    }

    SQInteger MovableObjectUserData::setLightDiffuseColour(HSQUIRRELVM vm){
        SQFloat floatValues[3];
        ScriptUtils::getFloatValues<3>(vm, 2, floatValues);

        Ogre::MovableObject* outObj = 0;
        SCRIPT_CHECK_RESULT(readMovableObjectFromUserData(vm, 1, &outObj, MovableObjectType::Light));

        Ogre::Light* lightObj = static_cast<Ogre::Light*>(outObj);

        lightObj->setDiffuseColour(floatValues[0], floatValues[1], floatValues[2]);

        return 0;
    }

    void MovableObjectUserData::setupDelegateTable(HSQUIRRELVM vm){
        { //Create item table
            sq_newtable(vm);

            ScriptUtils::addFunction(vm, setDatablock, "setDatablock", 2, ".u|s");

            sq_resetobject(&itemDelegateTableObject);
            sq_getstackobj(vm, -1, &itemDelegateTableObject);
            sq_addref(vm, &itemDelegateTableObject);
            sq_pop(vm, 1);
        }

        {
            sq_newtable(vm);

            ScriptUtils::addFunction(vm, setLightType, "setType", 2, ".i");
            ScriptUtils::addFunction(vm, setLightPowerScale, "setPowerScale", 2, ".n");
            ScriptUtils::addFunction(vm, setLightDiffuseColour, "setDiffuseColour", 4, ".nnn");

            sq_resetobject(&lightDelegateTableObject);
            sq_getstackobj(vm, -1, &lightDelegateTableObject);
            sq_addref(vm, &lightDelegateTableObject);
            sq_pop(vm, 1);
        }

    }

    void MovableObjectUserData::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_LIGHT_DIRECTIONAL", (SQInteger)Ogre::Light::LT_DIRECTIONAL);
        ScriptUtils::declareConstant(vm, "_LIGHT_POINT", (SQInteger)Ogre::Light::LT_POINT);
        ScriptUtils::declareConstant(vm, "_LIGHT_SPOTLIGHT", (SQInteger)Ogre::Light::LT_SPOTLIGHT);
        ScriptUtils::declareConstant(vm, "_LIGHT_VPL", (SQInteger)Ogre::Light::LT_VPL);
    }
}
