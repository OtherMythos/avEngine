#include "DatablockUserData.h"

#include "OgreHlmsDatablock.h"
#include "DatablockUnlitDelegate.h"
#include "DatablockPbsDelegate.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    SQObject DatablockUserData::pbsDelegateTableObject;
    SQObject DatablockUserData::unlitDelegateTableObject;

    SQInteger DatablockUserData::blockReleaseHook(SQUserPointer p, SQInteger size){

        return 0;
    }

    void DatablockUserData::DatablockPtrToUserData(HSQUIRRELVM vm, Ogre::HlmsDatablock* db){
        void* pointer = (void*)sq_newuserdata(vm, sizeof(Ogre::HlmsDatablock*));
        Ogre::HlmsDatablock** p = new (pointer) Ogre::HlmsDatablock*;
        *p = db;

        //sq_setreleasehook(vm, -1, blockReleaseHook);

        if(db->mType == Ogre::HLMS_PBS){
            sq_pushobject(vm, pbsDelegateTableObject);
        }else if(db->mType == Ogre::HLMS_UNLIT){
            sq_pushobject(vm, unlitDelegateTableObject);
        }else{
            assert(false); //Should not happen.
        }
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, datablockTypeTag);
    }

    SQInteger DatablockUserData::equalsDatablock(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db1 = 0;
        Ogre::HlmsDatablock* db2 = 0;

        SCRIPT_CHECK_RESULT(getPtrFromUserData(vm, -1, &db1));
        SCRIPT_CHECK_RESULT(getPtrFromUserData(vm, -1, &db2));

        sq_pushbool(vm, db1 == db2);

        return 1;
    }

    UserDataGetResult DatablockUserData::getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::HlmsDatablock** outPtr){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != datablockTypeTag){
            *outPtr = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        Ogre::HlmsDatablock** p = (Ogre::HlmsDatablock**)pointer;
        *outPtr = *p;

        return USER_DATA_GET_SUCCESS;
    }

    void DatablockUserData::setupDelegateTable(HSQUIRRELVM vm){
        ScriptUtils::setupDelegateTable(vm, &pbsDelegateTableObject, DatablockPbsDelegate::setupTable);
        ScriptUtils::setupDelegateTable(vm, &unlitDelegateTableObject, DatablockUnlitDelegate::setupTable);
    }
}
