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
        SQFloat x, y, z;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);

        Ogre::HlmsDatablock* db = getPtrFromUserData(vm, -4);
        assert(db->mType == Ogre::HLMS_PBS);

        Ogre::HlmsPbsDatablock* b = (Ogre::HlmsPbsDatablock*)db;
        b->setDiffuse(Ogre::Vector3(x, y, z));

        return 0;
    }

    SQInteger DatablockUserData::setMetalness(HSQUIRRELVM vm){
        SQFloat metalness;
        sq_getfloat(vm, -1, &metalness);

        Ogre::HlmsDatablock* db = getPtrFromUserData(vm, -2);
        assert(db->mType == Ogre::HLMS_PBS);

        Ogre::HlmsPbsDatablock* b = (Ogre::HlmsPbsDatablock*)db;
        b->setMetalness(metalness);

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

    Ogre::HlmsDatablock* DatablockUserData::getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx){
        SQUserPointer pointer;
        sq_getuserdata(vm, stackInx, &pointer, NULL);

        Ogre::HlmsDatablock** p = (Ogre::HlmsDatablock**)pointer;

        return *p;
    }

    void DatablockUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 2);

        sq_pushstring(vm, _SC("setDiffuse"), -1);
        sq_newclosure(vm, setDiffuse, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setMetalness"), -1);
        sq_newclosure(vm, setMetalness, 0);
        sq_newslot(vm, -3, false);

        sq_resetobject(&delegateTableObject);
        sq_getstackobj(vm, -1, &delegateTableObject);
        sq_addref(vm, &delegateTableObject);
        sq_pop(vm, 1);
    }
}
