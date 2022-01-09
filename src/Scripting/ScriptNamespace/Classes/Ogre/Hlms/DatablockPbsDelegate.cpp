#include "DatablockPbsDelegate.h"

#include "DatablockUserData.h"
#include "OgreHlmsPbsDatablock.h"
//#include "OgreTextureManager.h"

namespace AV{
    void DatablockPbsDelegate::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 7);

        ScriptUtils::addFunction(vm, setDiffuse, "setDiffuse", 4, ".nnn");
        ScriptUtils::addFunction(vm, setMetalness, "setMetalness", 2, ".n");
        ScriptUtils::addFunction(vm, setEmissive, "setEmissive", 4, ".nnn");
        ScriptUtils::addFunction(vm, setFresnel, "setFresnel", 4, ".nnn");

        ScriptUtils::addFunction(vm, setTransparency, "setTransparency", 5, ".nibb");

        ScriptUtils::addFunction(vm, getTransparency, "getTransparency");
        ScriptUtils::addFunction(vm, getTransparencyMode, "getTransparencyMode");
        ScriptUtils::addFunction(vm, getUseAlphaFromTextures, "getUseAlphaFromTextures");

        ScriptUtils::addFunction(vm, DatablockUserData::cloneDatablock, "cloneBlock", 2, ".s");
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

    SQInteger DatablockPbsDelegate::setTransparency(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        SQFloat transparency;
        sq_getfloat(vm, 2, &transparency);

        SQInteger transparencyMode;
        sq_getinteger(vm, 3, &transparencyMode);

        SQBool useAlphaFromTextures;
        sq_getbool(vm, 4, &useAlphaFromTextures);

        SQBool changeBlendblock;
        sq_getbool(vm, 5, &useAlphaFromTextures);

        b->setTransparency(transparency, static_cast<Ogre::HlmsPbsDatablock::TransparencyModes>(transparencyMode), useAlphaFromTextures, changeBlendblock);

        return 0;
    }

    SQInteger DatablockPbsDelegate::getTransparency(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        float trans = b->getTransparency();
        sq_pushfloat(vm, trans);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getUseAlphaFromTextures(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        bool texAlpha = b->getUseAlphaFromTextures();
        sq_pushbool(vm, texAlpha);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getTransparencyMode(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        Ogre::HlmsPbsDatablock::TransparencyModes mode = b->getTransparencyMode();
        SQInteger outInt = static_cast<SQInteger>(mode);
        sq_pushinteger(vm, outInt);

        return 1;
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
        SCRIPT_ASSERT_RESULT(DatablockUserData::getPtrFromUserData(vm, idx, &getDb));
        assert(getDb->mType == Ogre::HLMS_PBS);

        *db = (Ogre::HlmsPbsDatablock*)getDb;
    }


}
