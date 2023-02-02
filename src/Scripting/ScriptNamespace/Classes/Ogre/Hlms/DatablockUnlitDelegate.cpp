#include "DatablockUnlitDelegate.h"

#include "DatablockUserData.h"
#include "OgreHlmsUnlitDatablock.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "Scripting/ScriptNamespace/Classes/ColourValueUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"

namespace AV{
    void DatablockUnlitDelegate::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        ScriptUtils::addFunction(vm, setColour, "setColour", -2, ".u|nnnn");
        ScriptUtils::addFunction(vm, setUseColour, "setUseColour", 2, ".b");
        ScriptUtils::addFunction(vm, setTexture, "setTexture", 3, ".is|u");

        ScriptUtils::addFunction(vm, getColour, "getColour");

        ScriptUtils::addFunction(vm, DatablockUserData::cloneDatablock, "cloneBlock", 2, ".s");
        ScriptUtils::addFunction(vm, DatablockUserData::equalsDatablock, "equals", 2, ".u");

        ScriptUtils::addFunction(vm, getTypeof, "_typeof", 2, ".u");
    }

    SQInteger DatablockUnlitDelegate::getTypeof(HSQUIRRELVM vm){
        sq_pushstring(vm, "unlitDatablock", 14);
        return 1;
    }

    SQInteger DatablockUnlitDelegate::getColour(HSQUIRRELVM vm){
        Ogre::HlmsUnlitDatablock* db;
        _getUnitBlock(vm, &db, 1);

        Ogre::ColourValue val = db->getColour();
        ColourValueUserData::colourValueToUserData(vm, val);

        return 1;
    }

    SQInteger DatablockUnlitDelegate::setColour(HSQUIRRELVM vm){
        Ogre::ColourValue val;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read4FloatsOrColourValue(vm, &val));

        Ogre::HlmsUnlitDatablock* db;
        _getUnitBlock(vm, &db, 1);
        db->setColour(val);

        return 0;
    }

    SQInteger DatablockUnlitDelegate::setUseColour(HSQUIRRELVM vm){
        SQBool use;
        sq_getbool(vm, -1, &use);

        Ogre::HlmsUnlitDatablock* db;
        _getUnitBlock(vm, &db, 1);
        db->setUseColour(use);

        return 0;
    }

    SQInteger DatablockUnlitDelegate::setTexture(HSQUIRRELVM vm){
        SQInteger texType;
        sq_getinteger(vm, 2, &texType);

        if(texType < 0) texType = 0;
        if(texType > Ogre::NUM_UNLIT_TEXTURE_TYPES) texType = Ogre::NUM_UNLIT_TEXTURE_TYPES;

        Ogre::HlmsUnlitDatablock* db;
        _getUnitBlock(vm, &db, 1);

        if(sq_gettype(vm, 3) == OT_STRING){
            const SQChar *textureName;
            sq_getstring(vm, 3, &textureName);

            db->setTexture(texType, textureName);
        }else{
            Ogre::TextureGpu* outObject;
            bool userOwned, isValid;
            SCRIPT_CHECK_RESULT(TextureUserData::readTextureFromUserData(vm, 3, &outObject, &userOwned, &isValid));
            if(!isValid) return sq_throwerror(vm, "Texture is invalid.");

            db->setTexture(texType, outObject);
        }

        return 0;
    }

    void DatablockUnlitDelegate::_getUnitBlock(HSQUIRRELVM vm, Ogre::HlmsUnlitDatablock** db, SQInteger idx){
        Ogre::HlmsDatablock* getDb = 0;
        SCRIPT_ASSERT_RESULT(DatablockUserData::getPtrFromUserData(vm, idx, &getDb));
        assert(getDb->mType == Ogre::HLMS_UNLIT);

        *db = (Ogre::HlmsUnlitDatablock*)getDb;
    }
}
