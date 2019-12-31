#include "DatablockUserData.h"

#include "Logger/Log.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreTextureManager.h"

namespace AV{

    SQObject DatablockUserData::pbsDelegateTableObject;
    SQObject DatablockUserData::unlitDelegateTableObject;

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

    SQInteger DatablockUserData::setColour(HSQUIRRELVM vm){
        SQFloat x, y, z, w;
        sq_getfloat(vm, -1, &w);
        sq_getfloat(vm, -2, &z);
        sq_getfloat(vm, -3, &y);
        sq_getfloat(vm, -4, &x);

        Ogre::HlmsDatablock* db = getPtrFromUserData(vm, -5);
        assert(db->mType == Ogre::HLMS_UNLIT);

        Ogre::HlmsUnlitDatablock* b = (Ogre::HlmsUnlitDatablock*)db;
        b->setColour(Ogre::ColourValue(x, y, z, w));

        return 0;
    }

    SQInteger DatablockUserData::setUseColour(HSQUIRRELVM vm){
        SQBool use;
        sq_getbool(vm, -1, &use);

        Ogre::HlmsDatablock* db = getPtrFromUserData(vm, -2);
        assert(db->mType == Ogre::HLMS_UNLIT);

        Ogre::HlmsUnlitDatablock* b = (Ogre::HlmsUnlitDatablock*)db;
        b->setUseColour(use);

        return 0;
    }

    SQInteger DatablockUserData::setTexture(HSQUIRRELVM vm){
        SQInteger texType, arrayIndex;

        const SQChar *textureName;
        sq_getstring(vm, -1, &textureName);

        sq_getinteger(vm, -2, &arrayIndex);
        sq_getinteger(vm, -3, &texType);

        if(arrayIndex < 0) arrayIndex = 0;
        if(texType < 0) texType = 0;
        if(texType > Ogre::NUM_UNLIT_TEXTURE_TYPES) texType = Ogre::NUM_UNLIT_TEXTURE_TYPES;

        //TODO Might want to clean this up a bit with some checks.
        auto tex = Ogre::TextureManager::getSingleton().load(textureName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

        Ogre::HlmsDatablock* db = getPtrFromUserData(vm, -4);
        assert(db->mType == Ogre::HLMS_UNLIT);

        Ogre::HlmsUnlitDatablock* b = (Ogre::HlmsUnlitDatablock*)db;
        b->setTexture(texType, arrayIndex, tex);

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

    Ogre::HlmsDatablock* DatablockUserData::getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx){
        SQUserPointer pointer;
        sq_getuserdata(vm, stackInx, &pointer, NULL);

        Ogre::HlmsDatablock** p = (Ogre::HlmsDatablock**)pointer;

        return *p;
    }

    void DatablockUserData::setupDelegateTable(HSQUIRRELVM vm){
        //pbs
        sq_newtableex(vm, 2);

        ScriptUtils::addFunction(vm, setDiffuse, "setDiffuse");
        ScriptUtils::addFunction(vm, setMetalness, "setMetalness");

        sq_resetobject(&pbsDelegateTableObject);
        sq_getstackobj(vm, -1, &pbsDelegateTableObject);
        sq_addref(vm, &pbsDelegateTableObject);
        sq_pop(vm, 1);


        //unlit
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, setColour, "setColour");
        ScriptUtils::addFunction(vm, setUseColour, "setUseColour");
        ScriptUtils::addFunction(vm, setTexture, "setTexture");

        sq_resetobject(&unlitDelegateTableObject);
        sq_getstackobj(vm, -1, &unlitDelegateTableObject);
        sq_addref(vm, &unlitDelegateTableObject);
        sq_pop(vm, 1);
    }
}
