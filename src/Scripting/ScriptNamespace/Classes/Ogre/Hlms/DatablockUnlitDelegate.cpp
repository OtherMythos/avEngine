#include "DatablockUnlitDelegate.h"

#include "DatablockUserData.h"
#include "OgreHlmsUnlitDatablock.h"
//#include "OgreTextureManager.h"

namespace AV{
    void DatablockUnlitDelegate::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        ScriptUtils::addFunction(vm, setColour, "setColour", 5, ".nnnn");
        ScriptUtils::addFunction(vm, setUseColour, "setUseColour", 2, ".b");
        ScriptUtils::addFunction(vm, setTexture, "setTexture");

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
        SQInteger texType, arrayIndex;

        const SQChar *textureName;
        sq_getstring(vm, -1, &textureName);

        sq_getinteger(vm, -2, &arrayIndex);
        sq_getinteger(vm, -3, &texType);

        if(arrayIndex < 0) arrayIndex = 0;
        if(texType < 0) texType = 0;
        if(texType > Ogre::NUM_UNLIT_TEXTURE_TYPES) texType = Ogre::NUM_UNLIT_TEXTURE_TYPES;

        //TODO Might want to clean this up a bit with some checks.
        //auto tex = Ogre::TextureManager::getSingleton().load(textureName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

        //Ogre::HlmsDatablock* db = DatablockUserData::getPtrFromUserData(vm, -4);
        Ogre::HlmsUnlitDatablock* db;
        _getUnitBlock(vm, &db, -4);

        //db->setTexture(texType, arrayIndex, tex);
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
