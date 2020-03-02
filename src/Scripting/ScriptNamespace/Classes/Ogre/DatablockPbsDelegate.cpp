#include "DatablockPbsDelegate.h"

#include "DatablockUserData.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreTextureManager.h"

namespace AV{
    void DatablockPbsDelegate::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        ScriptUtils::addFunction(vm, setDiffuse, "setDiffuse", 4, ".nnn");
        ScriptUtils::addFunction(vm, setMetalness, "setMetalness", 2, ".n");
        ScriptUtils::addFunction(vm, setEmissive, "setEmissive", 4, ".nnn");
        ScriptUtils::addFunction(vm, setFresnel, "setFresnel", 4, ".nnn");

        ScriptUtils::addFunction(vm, DatablockUserData::equalsDatablock, "equals", 2, ".u");
    }

    SQInteger DatablockPbsDelegate::setDiffuse(HSQUIRRELVM vm){
        Ogre::Vector3 outVec;
        Ogre::HlmsPbsDatablock* b;
        _getVector3(vm, b, outVec);

        b->setDiffuse(outVec);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setEmissive(HSQUIRRELVM vm){
        Ogre::Vector3 outVec;
        Ogre::HlmsPbsDatablock* b;
        _getVector3(vm, b, outVec);

        b->setEmissive(outVec);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setFresnel(HSQUIRRELVM vm){
        Ogre::Vector3 outVec;
        Ogre::HlmsPbsDatablock* b;
        _getVector3(vm, b, outVec);

        //Not really tested what the boolean does so I'll just hard code it now.
        b->setFresnel(outVec, false);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setSpecular(HSQUIRRELVM vm){
        Ogre::Vector3 outVec;
        Ogre::HlmsPbsDatablock* b;
        _getVector3(vm, b, outVec);

        b->setSpecular(outVec);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setMetalness(HSQUIRRELVM vm){
        SQFloat metalness;
        sq_getfloat(vm, -1, &metalness);

        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, -2);
        b->setMetalness(metalness);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setRoughness(HSQUIRRELVM vm){
        SQFloat roughness;
        sq_getfloat(vm, -1, &roughness);

        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, -2);
        b->setMetalness(roughness);

        return 0;
    }

    void DatablockPbsDelegate::_getVector3(HSQUIRRELVM vm, Ogre::HlmsPbsDatablock*& db, Ogre::Vector3& vec){
        SQFloat x, y, z;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);

        _getPbsBlock(vm, &db, -4);
        vec = Ogre::Vector3(x, y, z);
    }

    void DatablockPbsDelegate::_getPbsBlock(HSQUIRRELVM vm, Ogre::HlmsPbsDatablock** db, SQInteger idx){
        Ogre::HlmsDatablock* getDb = 0;
        bool success = DatablockUserData::getPtrFromUserData(vm, idx, &getDb);
        assert(getDb->mType == Ogre::HLMS_PBS);

        *db = (Ogre::HlmsPbsDatablock*)getDb;
    }


}
