#include "DatablockUnlitDelegate.h"

#include "DatablockUserData.h"
#include "OgreHlmsUnlitDatablock.h"
//#include "OgreTextureManager.h"

namespace AV{
    void DatablockUnlitDelegate::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        ScriptUtils::addFunction(vm, setColour, "setColour", 5, ".nnnn");
        ScriptUtils::addFunction(vm, setUseColour, "setUseColour", 2, ".b");
        ScriptUtils::addFunction(vm, setTexture, "setTexture", 3, ".is");

        ScriptUtils::addFunction(vm, DatablockUserData::cloneDatablock, "cloneBlock", 2, ".s");
        ScriptUtils::addFunction(vm, DatablockUserData::equalsDatablock, "equals", 2, ".u");
    }

    SQInteger DatablockUnlitDelegate::setColour(HSQUIRRELVM vm){
        SQFloat x, y, z, w;
        sq_getfloat(vm, -1, &w);
        sq_getfloat(vm, -2, &z);
        sq_getfloat(vm, -3, &y);
        sq_getfloat(vm, -4, &x);

        Ogre::HlmsUnlitDatablock* db;
        _getUnitBlock(vm, &db, -5);
        db->setColour(Ogre::ColourValue(x, y, z, w));

        return 0;
    }

    SQInteger DatablockUnlitDelegate::setUseColour(HSQUIRRELVM vm){
        SQBool use;
        sq_getbool(vm, -1, &use);

        Ogre::HlmsUnlitDatablock* db;
        _getUnitBlock(vm, &db, -2);
        db->setUseColour(use);

        return 0;
    }

    SQInteger DatablockUnlitDelegate::setTexture(HSQUIRRELVM vm){
        SQInteger texType;
        sq_getinteger(vm, 2, &texType);

        const SQChar *textureName;
        sq_getstring(vm, 3, &textureName);

        if(texType < 0) texType = 0;
        if(texType > Ogre::NUM_UNLIT_TEXTURE_TYPES) texType = Ogre::NUM_UNLIT_TEXTURE_TYPES;

        Ogre::HlmsUnlitDatablock* db;
        _getUnitBlock(vm, &db, 1);

        db->setTexture(texType, textureName);

        return 0;
    }

    void DatablockUnlitDelegate::_getUnitBlock(HSQUIRRELVM vm, Ogre::HlmsUnlitDatablock** db, SQInteger idx){
        Ogre::HlmsDatablock* getDb = 0;
        SCRIPT_ASSERT_RESULT(DatablockUserData::getPtrFromUserData(vm, idx, &getDb));
        assert(getDb->mType == Ogre::HLMS_UNLIT);

        *db = (Ogre::HlmsUnlitDatablock*)getDb;
    }
}
