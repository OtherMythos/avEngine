#include "DatablockUserData.h"

#include "Logger/Log.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "OgreHlmsPbsDatablock.h"

namespace AV{

    SQObject DatablockUserData::delegateTableObject;

    SQInteger DatablockUserData::blockReleaseHook(SQUserPointer p, SQInteger size){

        return 0;
    }

    SQInteger DatablockUserData::setDiffuse(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db = getPtrFromUserData(vm);
        assert(db->mType == Ogre::HLMS_PBS);

        Ogre::HlmsPbsDatablock* b = (Ogre::HlmsPbsDatablock*)db;
        b->setDiffuse(Ogre::Vector3(10, 20, 30));

        return 0;
    }

    void DatablockUserData::DatablockPtrToUserData(HSQUIRRELVM vm, Ogre::HlmsDatablock* db){
        void* pointer = (void*)sq_newuserdata(vm, sizeof(Ogre::HlmsDatablock*));
        Ogre::HlmsDatablock** p = new (pointer) Ogre::HlmsDatablock*;
        *p = db;

        //sq_setreleasehook(vm, -1, blockReleaseHook);

        sq_pushobject(vm, delegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
    }

    Ogre::HlmsDatablock* DatablockUserData::getPtrFromUserData(HSQUIRRELVM vm){
        SQUserPointer pointer;
        sq_getuserdata(vm, -1, &pointer, NULL);

        Ogre::HlmsDatablock** p = (Ogre::HlmsDatablock**)pointer;

        return *p;
    }

    void DatablockUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 2);

        sq_pushstring(vm, _SC("setDiffuse"), -1);
        sq_newclosure(vm, setDiffuse, 0);
        sq_newslot(vm, -3, false);

        sq_resetobject(&delegateTableObject);
        sq_getstackobj(vm, -1, &delegateTableObject);
        sq_addref(vm, &delegateTableObject);
        sq_pop(vm, 1);
    }
}
