#include "MovableObjectUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"

#include "DatablockUserData.h"
#include "OgreMovableObject.h"
#include "OgreItem.h"

namespace AV{

    SQObject MovableObjectUserData::itemDelegateTableObject;

    void MovableObjectUserData::movableObjectToUserData(HSQUIRRELVM vm, Ogre::MovableObject* object){
        Ogre::MovableObject** pointer = (Ogre::MovableObject**)sq_newuserdata(vm, sizeof(Ogre::MovableObject*));
        *pointer = object;

        sq_pushobject(vm, itemDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, MovableObjectItemTypeTag);
    }

    UserDataGetResult MovableObjectUserData::readMovableObjectFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::MovableObject** outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != MovableObjectItemTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        Ogre::MovableObject** p = (Ogre::MovableObject**)pointer;
        *outObject = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger MovableObjectUserData::setDatablock(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_CHECK_RESULT(readMovableObjectFromUserData(vm, 1, &outObject));

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

    void MovableObjectUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, setDatablock, "setDatablock", 2, ".u|s");

        sq_resetobject(&itemDelegateTableObject);
        sq_getstackobj(vm, -1, &itemDelegateTableObject);
        sq_addref(vm, &itemDelegateTableObject);
        sq_pop(vm, 1);
    }
}
