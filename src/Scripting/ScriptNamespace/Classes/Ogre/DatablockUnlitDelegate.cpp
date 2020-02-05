#include "DatablockUnlitDelegate.h"

#include "DatablockUserData.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreTextureManager.h"

namespace AV{
    void DatablockUnlitDelegate::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 3);

        ScriptUtils::addFunction(vm, setColour, "setColour");
        ScriptUtils::addFunction(vm, setUseColour, "setUseColour");
        ScriptUtils::addFunction(vm, setTexture, "setTexture");

        ScriptUtils::addFunction(vm, DatablockUserData::equalsDatablock, "equals");
    }

    SQInteger DatablockUnlitDelegate::setColour(HSQUIRRELVM vm){
        SQFloat x, y, z, w;
        sq_getfloat(vm, -1, &w);
        sq_getfloat(vm, -2, &z);
        sq_getfloat(vm, -3, &y);
        sq_getfloat(vm, -4, &x);

        Ogre::HlmsDatablock* db = DatablockUserData::getPtrFromUserData(vm, -5);
        assert(db->mType == Ogre::HLMS_UNLIT);

        Ogre::HlmsUnlitDatablock* b = (Ogre::HlmsUnlitDatablock*)db;
        b->setColour(Ogre::ColourValue(x, y, z, w));

        return 0;
    }

    SQInteger DatablockUnlitDelegate::setUseColour(HSQUIRRELVM vm){
        SQBool use;
        sq_getbool(vm, -1, &use);

        Ogre::HlmsDatablock* db = DatablockUserData::getPtrFromUserData(vm, -2);
        assert(db->mType == Ogre::HLMS_UNLIT);

        Ogre::HlmsUnlitDatablock* b = (Ogre::HlmsUnlitDatablock*)db;
        b->setUseColour(use);

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
        auto tex = Ogre::TextureManager::getSingleton().load(textureName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

        Ogre::HlmsDatablock* db = DatablockUserData::getPtrFromUserData(vm, -4);
        assert(db->mType == Ogre::HLMS_UNLIT);

        Ogre::HlmsUnlitDatablock* b = (Ogre::HlmsUnlitDatablock*)db;
        b->setTexture(texType, arrayIndex, tex);

        return 0;
    }
}
