#include "DatablockPbsDelegate.h"

#include "Scripting/ScriptNamespace/Classes/Vector2UserData.h"
#include "OgreVector4.h"

#include "DatablockUserData.h"
#include "MacroblockUserData.h"
#include "BlendblockUserData.h"
#include "OgreHlmsPbsDatablock.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"
//#include "OgreTextureManager.h"

namespace AV{
    void DatablockPbsDelegate::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 14);

        ScriptUtils::addFunction(vm, setDiffuse, "setDiffuse", 4, ".nnn");
        ScriptUtils::addFunction(vm, setMetalness, "setMetalness", 2, ".n");
        ScriptUtils::addFunction(vm, setEmissive, "setEmissive", 4, ".nnn");
        ScriptUtils::addFunction(vm, setFresnel, "setFresnel", -4, ".nnnb");
        ScriptUtils::addFunction(vm, setNormalMapWeight, "setNormalMapWeight", 2, ".n");
        ScriptUtils::addFunction(vm, setIndexOfRefraction, "setIndexOfRefraction", -4, ".nnnb");
        ScriptUtils::addFunction(vm, setSpecular, "setSpecular", 4, ".nnn");
        ScriptUtils::addFunction(vm, setRoughness, "setRoughness", 2, ".n");
        ScriptUtils::addFunction(vm, setTexture, "setTexture", 3, ".is|o");
        ScriptUtils::addFunction(vm, setTextureUVSource, "setTextureUVSource", 3, ".ii");

        ScriptUtils::addFunction(vm, setWorkflow, "setWorkflow", 2, ".i");
        ScriptUtils::addFunction(vm, getWorkflow, "getWorkflow");

        ScriptUtils::addFunction(vm, setTransparency, "setTransparency", 5, ".nibb");

        ScriptUtils::addFunction(vm, getDiffuse, "getDiffuse");
        ScriptUtils::addFunction(vm, getTransparency, "getTransparency");
        ScriptUtils::addFunction(vm, getFresnel, "getFresnel");
        ScriptUtils::addFunction(vm, getMetalness, "getMetalness");
        ScriptUtils::addFunction(vm, getSpecular, "getSpecular");
        ScriptUtils::addFunction(vm, getEmissive, "getEmissive");
        ScriptUtils::addFunction(vm, getRoughness, "getRoughness");
        ScriptUtils::addFunction(vm, getNormalMapWeight, "getNormalMapWeight");
        ScriptUtils::addFunction(vm, getTransparencyMode, "getTransparencyMode");
        ScriptUtils::addFunction(vm, getUseAlphaFromTextures, "getUseAlphaFromTextures");
        ScriptUtils::addFunction(vm, hasSeparateFresnel, "hasSeparateFresnel");
        ScriptUtils::addFunction(vm, getTextureUVSource, "getTextureUVSource");
        ScriptUtils::addFunction(vm, getTexture, "getTexture", 2, ".i");

        ScriptUtils::addFunction(vm, getDetailMapBlendMode, "getDetailMapBlendMode", 2, ".i");
        ScriptUtils::addFunction(vm, getDetailMapOffset, "getDetailMapOffset", 2, "i");
        ScriptUtils::addFunction(vm, getDetailMapScale, "getDetailMapScale", 2, "i");
        ScriptUtils::addFunction(vm, getDetailMapWeight, "getDetailMapWeight", 2, "i");
        ScriptUtils::addFunction(vm, getDetailNormalMapWeight, "getDetailNormalMapWeight", 2, "i");

        ScriptUtils::addFunction(vm, setDetailMapBlendMode, "setDetailMapBlendMode", 3, ".ii");
        ScriptUtils::addFunction(vm, setDetailMapOffset, "setDetailMapOffset", 3, ".iu");
        ScriptUtils::addFunction(vm, setDetailMapScale, "setDetailMapScale", 3, ".iu");
        ScriptUtils::addFunction(vm, setDetailMapWeight, "setDetailMapWeight", 3, ".in");
        ScriptUtils::addFunction(vm, setDetailNormalMapWeight, "setDetailNormalMapWeight", 3, ".in");

        ScriptUtils::addFunction(vm, DatablockUserData::cloneDatablock, "cloneBlock", 2, ".s");
        ScriptUtils::addFunction(vm, DatablockUserData::equalsDatablock, "equals", 2, ".u");
        ScriptUtils::addFunction(vm, setMacroblock, "setMacroblock", -2, ".ub");
        ScriptUtils::addFunction(vm, setBlendblock, "setBlendblock", -2, ".ub");

        ScriptUtils::addFunction(vm, setShadowConstBias, "setShadowConstBias", 2, ".n");
        ScriptUtils::addFunction(vm, getShadowConstBias, "getShadowConstBias");

        ScriptUtils::addFunction(vm, getTypeof, "_typeof");
    }

    SQInteger DatablockPbsDelegate::getTypeof(HSQUIRRELVM vm){
        sq_pushstring(vm, "pbsDatablock", 12);
        return 1;
    }

    SQInteger DatablockPbsDelegate::getWorkflow(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        Ogre::HlmsPbsDatablock::Workflows w = b->getWorkflow();
        SQInteger intVal = static_cast<SQInteger>(w);
        sq_pushinteger(vm, intVal);

        return 1;
    }

    SQInteger DatablockPbsDelegate::setWorkflow(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        SQInteger value;
        sq_getinteger(vm, 2, &value);
        if(
            value != Ogre::HlmsPbsDatablock::Workflows::SpecularWorkflow &&
            value != Ogre::HlmsPbsDatablock::Workflows::SpecularAsFresnelWorkflow &&
            value != Ogre::HlmsPbsDatablock::Workflows::MetallicWorkflow
        ){
            return sq_throwerror(vm, "Invalid workflow provided.");
        }

        Ogre::HlmsPbsDatablock::Workflows workflow = static_cast<Ogre::HlmsPbsDatablock::Workflows>(value);
        b->setWorkflow(workflow);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setMacroblock(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        const Ogre::HlmsMacroblock* out;
        SCRIPT_CHECK_RESULT(MacroblockUserData::getPtrFromUserData(vm, 2, &out));

        SQBool casterBlock = false;
        if(sq_gettop(vm) == 3){
            sq_getbool(vm, 3, &casterBlock);
        }

        b->setMacroblock(out, casterBlock);

        return 0;
    }

    SQInteger DatablockPbsDelegate::getDetailMapBlendMode(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        Ogre::uint8 detailId;
        SQInteger result = _getPbsBlockAndDetailId(vm, &b, &detailId, 1);
        if(result != 0) return result;

        Ogre::PbsBlendModes blendMode = b->getDetailMapBlendMode(detailId);

        sq_pushinteger(vm, static_cast<SQInteger>(blendMode));

        return 1;
    }

    SQInteger DatablockPbsDelegate::getDetailMapOffset(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        Ogre::uint8 detailId;
        SQInteger result = _getPbsBlockAndDetailId(vm, &b, &detailId, 1);
        if(result != 0) return result;

        Ogre::Vector4 offsetScale = b->getDetailMapOffsetScale(detailId);

        Vector2UserData::vector2ToUserData(vm, offsetScale.xy());

        return 1;
    }

    SQInteger DatablockPbsDelegate::getDetailMapScale(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        Ogre::uint8 detailId;
        SQInteger result = _getPbsBlockAndDetailId(vm, &b, &detailId, 1);
        if(result != 0) return result;

        Ogre::Vector4 offsetScale = b->getDetailMapOffsetScale(detailId);

        Ogre::Vector2 outValue(offsetScale.z, offsetScale.w);
        Vector2UserData::vector2ToUserData(vm, outValue);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getDetailMapWeight(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        Ogre::uint8 detailId;
        SQInteger result = _getPbsBlockAndDetailId(vm, &b, &detailId, 1);
        if(result != 0) return result;

        Ogre::Real weight = b->getDetailMapWeight(detailId);

        sq_pushfloat(vm, weight);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getDetailNormalMapWeight(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        Ogre::uint8 detailId;
        SQInteger result = _getPbsBlockAndDetailId(vm, &b, &detailId, 1);
        if(result != 0) return result;

        Ogre::Real weight = b->getDetailNormalWeight(detailId);

        sq_pushfloat(vm, weight);

        return 1;
    }

    SQInteger DatablockPbsDelegate::setDetailMapBlendMode(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        Ogre::uint8 detailId;
        SQInteger result = _getPbsBlockAndDetailId(vm, &b, &detailId, 1);
        if(result != 0) return result;

        SQInteger blendMode;
        sq_getinteger(vm, 2, &blendMode);

        Ogre::PbsBlendModes mode = static_cast<Ogre::PbsBlendModes>(blendMode);
        b->setDetailMapBlendMode(detailId, mode);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setDetailMapOffset(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        Ogre::uint8 detailId;
        SQInteger result = _getPbsBlockAndDetailId(vm, &b, &detailId, 1);
        if(result != 0) return result;

        Ogre::Vector2 outVec;
        Vector2UserData::readVector2FromUserData(vm, 3, &outVec);

        Ogre::Vector4 offsetScale = b->getDetailMapOffsetScale(detailId);
        offsetScale.x = outVec.x;
        offsetScale.y = outVec.y;
        b->setDetailMapOffsetScale(detailId, offsetScale);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setDetailMapScale(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        Ogre::uint8 detailId;
        SQInteger result = _getPbsBlockAndDetailId(vm, &b, &detailId, 1);
        if(result != 0) return result;

        Ogre::Vector2 outVec;
        Vector2UserData::readVector2FromUserData(vm, 3, &outVec);

        Ogre::Vector4 offsetScale = b->getDetailMapOffsetScale(detailId);
        offsetScale.z = outVec.x;
        offsetScale.w = outVec.y;
        b->setDetailMapOffsetScale(detailId, offsetScale);

        return 0;
    }


    SQInteger DatablockPbsDelegate::setDetailMapWeight(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        Ogre::uint8 detailId;
        SQInteger result = _getPbsBlockAndDetailId(vm, &b, &detailId, 1);
        if(result != 0) return result;

        SQFloat weight;
        sq_getfloat(vm, 3, &weight);

        b->setDetailMapWeight(detailId, weight);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setDetailNormalMapWeight(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        Ogre::uint8 detailId;
        SQInteger result = _getPbsBlockAndDetailId(vm, &b, &detailId, 1);
        if(result != 0) return result;

        SQFloat weight;
        sq_getfloat(vm, 3, &weight);

        b->setDetailNormalWeight(detailId, weight);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setShadowConstBias(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        SQFloat value;
        sq_getfloat(vm, 2, &value);

        b->mShadowConstantBias = value;

        return 0;
    }
    SQInteger DatablockPbsDelegate::getShadowConstBias(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        sq_pushfloat(vm, b->mShadowConstantBias);

        return 1;
    }

    SQInteger DatablockPbsDelegate::setBlendblock(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        const Ogre::HlmsBlendblock* out;
        SCRIPT_CHECK_RESULT(BlendblockUserData::getPtrFromUserData(vm, 2, &out));

        SQBool casterBlock = false;
        if(sq_gettop(vm) == 3){
            sq_getbool(vm, 3, &casterBlock);
        }

        b->setBlendblock(out, casterBlock);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setTextureUVSource(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);
        SQInteger value, uvValue;
        sq_getinteger(vm, 2, &value);
        sq_getinteger(vm, 3, &uvValue);

        if(value >= Ogre::NUM_PBSM_TEXTURE_TYPES) sq_throwerror(vm, "Invalid texture type");

        Ogre::PbsTextureTypes t = static_cast<Ogre::PbsTextureTypes>(value);
        b->setTextureUvSource(t, uvValue);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setTexture(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);
        SQInteger value;
        sq_getinteger(vm, 2, &value);
        const SQChar *text = 0;
        //If null is provided then set the texture to nothing.
        if(sq_gettype(vm, 3) == OT_STRING){
            sq_getstring(vm, 3, &text);
        }

        if(value >= Ogre::NUM_PBSM_TEXTURE_TYPES) sq_throwerror(vm, "Invalid texture type");

        Ogre::PbsTextureTypes t = static_cast<Ogre::PbsTextureTypes>(value);
        if(text == 0){
            b->setTexture(static_cast<Ogre::uint8>(value), 0);
        }else{
            b->setTexture(t, text);
        }

        return 0;
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
        _getVector3(vm, b, outVec, -1);

        SQBool separateFresnel = false;
        if(sq_gettop(vm) == 5){
            sq_getbool(vm, -1, &separateFresnel);
        }

        b->setFresnel(outVec, separateFresnel);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setIndexOfRefraction(HSQUIRRELVM vm){
        Ogre::Vector3 outVec;
        Ogre::HlmsPbsDatablock* b;
        _getVector3(vm, b, outVec, -1);

        SQBool separateFresnel = false;
        if(sq_gettop(vm) == 5){
            sq_getbool(vm, -1, &separateFresnel);
        }

        b->setIndexOfRefraction(outVec, separateFresnel);

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

    SQInteger DatablockPbsDelegate::setNormalMapWeight(HSQUIRRELVM vm){
        SQFloat weight;
        sq_getfloat(vm, -1, &weight);

        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, -2);
        b->setNormalMapWeight(weight);

        return 0;
    }

    SQInteger DatablockPbsDelegate::setRoughness(HSQUIRRELVM vm){
        SQFloat roughness;
        sq_getfloat(vm, -1, &roughness);

        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, -2);
        b->setRoughness(roughness);

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
        sq_getbool(vm, 5, &changeBlendblock);

        b->setTransparency(transparency, static_cast<Ogre::HlmsPbsDatablock::TransparencyModes>(transparencyMode), useAlphaFromTextures, changeBlendblock);

        return 0;
    }

    SQInteger DatablockPbsDelegate::getNormalMapWeight(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        float norm = b->getNormalMapWeight();
        sq_pushfloat(vm, norm);

        return 1;
    }


    SQInteger DatablockPbsDelegate::getTransparency(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        float trans = b->getTransparency();
        sq_pushfloat(vm, trans);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getDiffuse(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        Ogre::Vector3 diffuse = b->getDiffuse();
        Vector3UserData::vector3ToUserData(vm, diffuse);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getFresnel(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        Ogre::Vector3 fresnel = b->getFresnel();
        Vector3UserData::vector3ToUserData(vm, fresnel);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getMetalness(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        float fresnel = b->getMetalness();
        sq_pushfloat(vm, fresnel);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getSpecular(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        Ogre::Vector3 specular = b->getSpecular();
        Vector3UserData::vector3ToUserData(vm, specular);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getEmissive(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        Ogre::Vector3 emissive = b->getEmissive();
        Vector3UserData::vector3ToUserData(vm, emissive);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getRoughness(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        float roughness = b->getRoughness();
        sq_pushfloat(vm, roughness);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getUseAlphaFromTextures(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        bool texAlpha = b->getUseAlphaFromTextures();
        sq_pushbool(vm, texAlpha);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getTexture(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        SQInteger texType;
        sq_getinteger(vm, 2, &texType);

        if(texType >= Ogre::NUM_PBSM_TEXTURE_TYPES) sq_throwerror(vm, "Invalid texture type");

        Ogre::PbsTextureTypes t = static_cast<Ogre::PbsTextureTypes>(texType);
        Ogre::TextureGpu* tex = b->getTexture(t);
        TextureUserData::textureToUserData(vm, tex, true);

        return 1;
    }

    SQInteger DatablockPbsDelegate::getTextureUVSource(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        SQInteger texType;
        sq_getinteger(vm, 2, &texType);

        if(texType >= Ogre::NUM_PBSM_TEXTURE_TYPES) sq_throwerror(vm, "Invalid texture type");

        Ogre::PbsTextureTypes t = static_cast<Ogre::PbsTextureTypes>(texType);
        Ogre::uint8 texVal = b->getTextureUvSource(t);
        sq_pushinteger(vm, static_cast<SQInteger>(texVal));

        return 1;
    }

    SQInteger DatablockPbsDelegate::hasSeparateFresnel(HSQUIRRELVM vm){
        Ogre::HlmsPbsDatablock* b;
        _getPbsBlock(vm, &b, 1);

        bool fres = b->hasSeparateFresnel();
        sq_pushbool(vm, fres);

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

    void DatablockPbsDelegate::_getVector3(HSQUIRRELVM vm, Ogre::HlmsPbsDatablock*& db, Ogre::Vector3& vec, int start){
        SQFloat x, y, z;
        sq_getfloat(vm, -1 + start, &z);
        sq_getfloat(vm, -2 + start, &y);
        sq_getfloat(vm, -3 + start, &x);

        _getPbsBlock(vm, &db, -4 + start);
        vec = Ogre::Vector3(x, y, z);
    }

    void DatablockPbsDelegate::_getPbsBlock(HSQUIRRELVM vm, Ogre::HlmsPbsDatablock** db, SQInteger idx){
        Ogre::HlmsDatablock* getDb = 0;
        SCRIPT_ASSERT_RESULT(DatablockUserData::getPtrFromUserData(vm, idx, &getDb));
        assert(getDb->mType == Ogre::HLMS_PBS);

        *db = (Ogre::HlmsPbsDatablock*)getDb;
    }

    SQInteger DatablockPbsDelegate::_getPbsBlockAndDetailId(HSQUIRRELVM vm, Ogre::HlmsPbsDatablock** db, Ogre::uint8* detailId, SQInteger idx){
        _getPbsBlock(vm, db, idx);

        SQInteger id;
        sq_getinteger(vm, idx + 1, &id);
        if(id < 0 || id >= 4) return sq_throwerror(vm, "Index must be in range 0-3");
        *detailId = id;

        return 0;
    }
}
