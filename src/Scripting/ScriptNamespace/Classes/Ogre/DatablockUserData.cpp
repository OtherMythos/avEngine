#include "DatablockUserData.h"

#include "OgreHlmsDatablock.h"
#include "DatablockUnlitDelegate.h"
#include "DatablockPbsDelegate.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

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
    }

    SQInteger DatablockUserData::equalsDatablock(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db1 = getPtrFromUserData(vm, -1);
        Ogre::HlmsDatablock* db2 = getPtrFromUserData(vm, -2);

        sq_pushbool(vm, db1 == db2);

        return 1;
    }

    Ogre::HlmsDatablock* DatablockUserData::getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx){
        SQUserPointer pointer;
        sq_getuserdata(vm, stackInx, &pointer, NULL);

        Ogre::HlmsDatablock** p = (Ogre::HlmsDatablock**)pointer;

        return *p;
    }

    void DatablockUserData::setupDelegateTable(HSQUIRRELVM vm){
        //pbs
        DatablockPbsDelegate::setupTable(vm);

        sq_resetobject(&pbsDelegateTableObject);
        sq_getstackobj(vm, -1, &pbsDelegateTableObject);
        sq_addref(vm, &pbsDelegateTableObject);
        sq_pop(vm, 1);


        //unlit

        DatablockUnlitDelegate::setupTable(vm);

        sq_resetobject(&unlitDelegateTableObject);
        sq_getstackobj(vm, -1, &unlitDelegateTableObject);
        sq_addref(vm, &unlitDelegateTableObject);
        sq_pop(vm, 1);
    }
}
