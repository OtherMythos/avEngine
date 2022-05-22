#include "HlmsNamespace.h"

#include "Logger/Log.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Hlms/DatablockUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Hlms/MacroblockUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Hlms/BlendblockUserData.h"

#include "Ogre.h"
#include "OgreHlms.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsUnlit.h"
#include "OgreHlmsPbsDatablock.h"

namespace AV {

    void _populateDatablockConstructionInfo(HSQUIRRELVM vm, SQInteger idx, Ogre::HlmsParamVec& vec){
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, idx))){
            SQObjectType t = sq_gettype(vm, -1);
            if(t != OT_STRING){
                sq_pop(vm,2); //We only really care about strings.
                continue;
            }

            //here -1 is the value and -2 is the key
            const SQChar *k, *v;
            sq_getstring(vm, -2, &k);
            sq_getstring(vm, -1, &v);

            vec.push_back({k, v});

            sq_pop(vm,2);
        }

        sq_pop(vm,1);
    }

    template<class HlmsType>
    SQInteger _createDatablock(HSQUIRRELVM vm, Ogre::HlmsTypes type){
        SQInteger top = sq_gettop(vm);

        const SQChar *dbName;
        const Ogre::HlmsMacroblock* targetMacroblock = 0;
        const Ogre::HlmsBlendblock* targetBlendblock = 0;

        sq_getstring(vm, 2, &dbName);

        //Get blendblock.
        if(top >= 3){
            SQObjectType t = sq_gettype(vm, 3);
            if(t != OT_NULL){ //Must be a user data if it's not null.
                assert(t == OT_USERDATA);
                bool success = BlendblockUserData::getPtrFromUserData(vm, 3, &targetBlendblock);
                if(!success){
                    return sq_throwerror(vm, "Incorrect object passed as blendblock");
                }
            }
        }
        //Get macroblock.
        if(top >= 4){
            SQObjectType t = sq_gettype(vm, 4);
            if(t != OT_NULL){
                assert(t == OT_USERDATA);
                bool success = MacroblockUserData::getPtrFromUserData(vm, 4, &targetMacroblock);
                if(!success){
                    return sq_throwerror(vm, "Incorrect object passed as macroblock");
                }
            }
        }
        Ogre::HlmsParamVec vec;
        if(top >= 5){
            SQObjectType t = sq_gettype(vm, 5);
            if(t != OT_NULL){
                assert(t == OT_TABLE);
                _populateDatablockConstructionInfo(vm, 5, vec);
            }
        }

        Ogre::HlmsDatablock* newBlock = 0;
        {
            using namespace Ogre;
            Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(type);
            HlmsType* foundHlms = dynamic_cast<HlmsType*>(hlms);

            try{
                newBlock = foundHlms->createDatablock(Ogre::IdString(dbName), dbName,
                   targetMacroblock != 0 ? *targetMacroblock : Ogre::HlmsMacroblock(),
                   targetBlendblock != 0 ? *targetBlendblock : Ogre::HlmsBlendblock(),
                vec);
            }catch(Ogre::ItemIdentityException e){
                return sq_throwerror(vm, "Datablock name exists");
            }

        }

        if(!newBlock){
            return 0;
        }

        DatablockUserData::DatablockPtrToUserData(vm, (Ogre::HlmsDatablock*)newBlock);
        return 1;
    }

    SQInteger HlmsNamespace::PBSCreateDatablock(HSQUIRRELVM vm){
        return _createDatablock<Ogre::HlmsPbs>(vm, Ogre::HLMS_PBS);
    }

    SQInteger HlmsNamespace::_getDefaultDatablock(HSQUIRRELVM vm, Ogre::HlmsTypes type){
        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(type);
        Ogre::HlmsDatablock* block = hlms->getDefaultDatablock();
        if(!block){
            return 0;
        }

        DatablockUserData::DatablockPtrToUserData(vm, block);
        return 1;
    }

    SQInteger HlmsNamespace::PBSGetDefaultDatablock(HSQUIRRELVM vm){
        return _getDefaultDatablock(vm, Ogre::HLMS_PBS);
    }

    SQInteger HlmsNamespace::UnlitGetDefaultDatablock(HSQUIRRELVM vm){
        return _getDefaultDatablock(vm, Ogre::HLMS_UNLIT);
    }

    SQInteger HlmsNamespace::UnlitCreateDatablock(HSQUIRRELVM vm){
        return _createDatablock<Ogre::HlmsUnlit>(vm, Ogre::HLMS_UNLIT);
    }

    void HlmsNamespace::_parseMacroblockConstructionInfo(HSQUIRRELVM vm, Ogre::HlmsMacroblock* block){
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm,-2))){
            //here -1 is the value and -2 is the key
            const SQChar *k;
            sq_getstring(vm, -2, &k);

            SQObjectType t = sq_gettype(vm, -1);
            if(t == OT_BOOL){
                SQBool val;
                sq_getbool(vm, -1, &val);
                if(strcmp(k, "depthCheck") == 0){
                    block->mDepthCheck = val;
                }else if(strcmp(k, "depthWrite") == 0){
                    block->mDepthWrite = val;
                }else if(strcmp(k, "scissorTestEnabled") == 0){
                    block->mScissorTestEnabled = val;
                }
            }
            else if(t == OT_INTEGER){
                SQInteger val;
                sq_getinteger(vm, -1, &val);
                if(strcmp(k, "polygonMode") == 0){
                    //Polygon mode starts at 1
                    block->mPolygonMode = (Ogre::PolygonMode)(val + 1);
                }else if(strcmp(k, "cullMode") == 0){
                    //So does this one.
                    block->mCullMode = (Ogre::CullingMode)(val + 1);
                }else if(strcmp(k, "depthFunction") == 0){
                    block->mDepthFunc = (Ogre::CompareFunction)val;
                }
            }
            else if(t == OT_INTEGER || t == OT_FLOAT){
                SQFloat val;
                sq_getfloat(vm, -1, &val);
                if(strcmp(k, "depthBiasConstant") == 0){
                    block->mDepthBiasConstant = val;
                }else if(strcmp(k, "depthBiasSlopeScale") == 0){
                    block->mDepthBiasSlopeScale = val;
                }
            }

            sq_pop(vm,2); //pop the key and value
        }

        sq_pop(vm,1); //pops the null iterator
    }

    SQInteger HlmsNamespace::getMacroblock(HSQUIRRELVM vm){
        Ogre::HlmsMacroblock macro;
        _parseMacroblockConstructionInfo(vm, &macro);

        const Ogre::HlmsMacroblock* createdBlock = Ogre::Root::getSingletonPtr()->getHlmsManager()->getMacroblock(macro);

        MacroblockUserData::MacroblockPtrToUserData(vm, createdBlock);

        return 1;
    }

    bool HlmsNamespace::_getSceneBlendFactor(SQInteger value, Ogre::SceneBlendFactor* out){
        if(value < 0 || value > Ogre::SBF_ONE_MINUS_SOURCE_ALPHA) return false;
        *out = static_cast<Ogre::SceneBlendFactor>(value);
        return true;
    }

    bool HlmsNamespace::_getSceneBlendOperation(SQInteger value, Ogre::SceneBlendOperation* out){
        if(value < 0 || value > Ogre::SBO_MAX) return false;
        *out = static_cast<Ogre::SceneBlendOperation>(value);
        return true;
    }

    void HlmsNamespace::_parseBlendblockConstructionInfo(HSQUIRRELVM vm, Ogre::HlmsBlendblock* block){
        sq_pushnull(vm);
        bool failed = false;
        while(SQ_SUCCEEDED(sq_next(vm,-2))){
            //here -1 is the value and -2 is the key
            const SQChar *k;
            sq_getstring(vm, -2, &k);

            SQObjectType t = sq_gettype(vm, -1);
            if(t == OT_BOOL){
                SQBool val;
                sq_getbool(vm, -1, &val);
                if(strcmp(k, "alpha_to_coverage") == 0){
                    block->mAlphaToCoverageEnabled = val;
                }else if(strcmp(k, "separate_blend") == 0){
                    block->mSeparateBlend = val;
                }
            }
            else if(t == OT_INTEGER){
#define READ_SCENE_BLEND_FACTOR(__x) Ogre::SceneBlendFactor factor; bool __result__ = _getSceneBlendFactor(val, &factor); if(__result__) block->__x = factor; else failed = true;
                SQInteger val;
                sq_getinteger(vm, -1, &val);
                if(strcmp(k, "src_blend_factor") == 0){
                    READ_SCENE_BLEND_FACTOR(mSourceBlendFactor);
                }else if(strcmp(k, "dst_blend_factor") == 0){
                    READ_SCENE_BLEND_FACTOR(mDestBlendFactor);
                }else if(strcmp(k, "src_alpha_blend_factor") == 0){
                    READ_SCENE_BLEND_FACTOR(mSourceBlendFactorAlpha);
                }else if(strcmp(k, "dst_alpha_blend_factor") == 0){
                    READ_SCENE_BLEND_FACTOR(mDestBlendFactorAlpha);
                }
#undef READ_SCENE_BLEND_FACTOR

#define READ_SCENE_BLEND_OP(__x) Ogre::SceneBlendOperation op; bool __result__ = _getSceneBlendOperation(val, &op); if(__result__) block->__x = op; else failed = true;
                else if(strcmp(k, "blend_operation") == 0){
                    READ_SCENE_BLEND_OP(mBlendOperation);
                }
                else if(strcmp(k, "blend_operation_alpha") == 0){
                    READ_SCENE_BLEND_OP(mBlendOperationAlpha);
                }
            }
#undef READ_SCENE_BLEND_OP

            sq_pop(vm,2); //pop the key and value
        }

        sq_pop(vm,1); //pops the null iterator
    }

    SQInteger HlmsNamespace::getBlendblock(HSQUIRRELVM vm){
        Ogre::HlmsBlendblock blend;
        _parseBlendblockConstructionInfo(vm, &blend);

        const Ogre::HlmsBlendblock* createdBlock = Ogre::Root::getSingletonPtr()->getHlmsManager()->getBlendblock(blend);

        BlendblockUserData::BlendblockPtrToUserData(vm, createdBlock);

        return 1;
    }

    SQInteger HlmsNamespace::getDatablock(HSQUIRRELVM vm){
        const SQChar *dbName;
        sq_getstring(vm, -1, &dbName);

        Ogre::HlmsDatablock* db = Ogre::Root::getSingletonPtr()->getHlmsManager()->getDatablockNoDefault(dbName);

        if(!db){
            return 0; //Returns null
        }

        DatablockUserData::DatablockPtrToUserData(vm, db);
        return 1;
    }

    SQInteger HlmsNamespace::destroyDatablock(HSQUIRRELVM vm){

        SQObjectType t = sq_gettype(vm, -1);
        Ogre::HlmsDatablock* db = 0;
        Ogre::HlmsManager* mgr = Ogre::Root::getSingletonPtr()->getHlmsManager();
        if(t == OT_USERDATA){
            SCRIPT_CHECK_RESULT(DatablockUserData::getPtrFromUserData(vm, -1, &db));
        }else if(t == OT_STRING){
            const SQChar *dbName;
            sq_getstring(vm, -1, &dbName);

            db = mgr->getDatablockNoDefault(dbName);

            if(!db){ //No actual datablock was found, so there was nothing to delete.
                return sq_throwerror(vm, "No datablock found.");
            }
        }else assert(false);

        assert(db);

        Ogre::Hlms* hlms = mgr->getHlms( static_cast<Ogre::HlmsTypes>(db->mType));
        assert(hlms);

        if(db->mType == Ogre::HLMS_PBS){
            Ogre::HlmsPbs* pbsHlms = dynamic_cast<Ogre::HlmsPbs*>(hlms);
            //These functions do throw, but by this point it shouldn't have any issues so I don't check them.
            pbsHlms->destroyDatablock(db->getName());
        }else if(db->mType == Ogre::HLMS_UNLIT){
            Ogre::HlmsUnlit* unlitHlms = dynamic_cast<Ogre::HlmsUnlit*>(hlms);
            unlitHlms->destroyDatablock(db->getName());
        }else assert(false);

        return 0;
    }

    /**SQNamespace
    @name _hlms
    @desc This namespace provides mechanisms to interact with Ogre's HLMS (High Level Material System). Here, what are generally thought of as materials are named datablocks. Datablocks contain information relating to how objects should be rendered, so they can be thought of as materials.
    */
    void HlmsNamespace::setupNamespace(HSQUIRRELVM vm){
        //pbs
        {
            sq_pushstring(vm, _SC("pbs"), -1);
            sq_newtableex(vm, 2);

            /**
            createDatablock(string datablockName, Macroblock block = null, table constructionParams = {});
            */
            //string, blendblock, macroblock, constructor
            ScriptUtils::addFunction(vm, PBSCreateDatablock, "createDatablock", -2, ".s u|o u|o t|o");
            ScriptUtils::addFunction(vm, PBSGetDefaultDatablock, "getDefaultDatablock");

            sq_newslot(vm,-3,SQFalse);
        }

        //unlit
        {
            sq_pushstring(vm, _SC("unlit"), -1);
            sq_newtableex(vm, 2);

            ScriptUtils::addFunction(vm, UnlitCreateDatablock, "createDatablock", -2, ".s u|o u|o t|o");//string, blendblock, macroblock, construction info
            ScriptUtils::addFunction(vm, UnlitGetDefaultDatablock, "getDefaultDatablock");

            sq_newslot(vm,-3,SQFalse);
        }

        /**SQFunction
        @name getDatablock
        @param1:datablockName: A string representing the datablock name.
        @desc Retreives a pre-existing datablock from Ogre.
        @returns A handle to a datablock. If the datablock was not found null will be returned.
        */
        ScriptUtils::addFunction(vm, getDatablock, "getDatablock", 2, ".s");
        /**SQFunction
        @name destroyDatablock
        @param1:datablock: Either a string name, or a datablock handle.
        @desc Destroys a pre-existing datablock.
        */
        ScriptUtils::addFunction(vm, destroyDatablock, "destroyDatablock", 2, ". s|u");

        /**SQFunction
        @name getMacroblock
        @param1:table: A table containing the construction info for that macroblock.
        @desc Get a reference to a macroblock with some construction parameters.
        */
        ScriptUtils::addFunction(vm, getMacroblock, "getMacroblock", 2, ".t");
        /**SQFunction
        @name getBlendblock
        @param1:table: A table containing the construction info for that blendblock.
        @desc Get a reference to a blendblock with some construction parameters.
        */
        ScriptUtils::addFunction(vm, getBlendblock, "getBlendblock", 2, ".t");

    }

    void HlmsNamespace::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_PBSM_DIFFUSE", Ogre::PBSM_DIFFUSE);
        ScriptUtils::declareConstant(vm, "_PBSM_NORMAL", Ogre::PBSM_NORMAL);
        ScriptUtils::declareConstant(vm, "_PBSM_SPECULAR", Ogre::PBSM_SPECULAR);
        ScriptUtils::declareConstant(vm, "_PBSM_METALLIC", Ogre::PBSM_METALLIC);
        ScriptUtils::declareConstant(vm, "_PBSM_ROUGHNESS", Ogre::PBSM_ROUGHNESS);
        ScriptUtils::declareConstant(vm, "_PBSM_DETAIL_WEIGHT", Ogre::PBSM_DETAIL_WEIGHT);
        ScriptUtils::declareConstant(vm, "_PBSM_DETAIL0", Ogre::PBSM_DETAIL0);
        ScriptUtils::declareConstant(vm, "_PBSM_DETAIL1", Ogre::PBSM_DETAIL1);
        ScriptUtils::declareConstant(vm, "_PBSM_DETAIL2", Ogre::PBSM_DETAIL2);
        ScriptUtils::declareConstant(vm, "_PBSM_DETAIL3", Ogre::PBSM_DETAIL3);
        ScriptUtils::declareConstant(vm, "_PBSM_DETAIL0_NM", Ogre::PBSM_DETAIL0_NM);
        ScriptUtils::declareConstant(vm, "_PBSM_DETAIL1_NM", Ogre::PBSM_DETAIL1_NM);
        ScriptUtils::declareConstant(vm, "_PBSM_DETAIL2_NM", Ogre::PBSM_DETAIL2_NM);
        ScriptUtils::declareConstant(vm, "_PBSM_DETAIL3_NM", Ogre::PBSM_DETAIL3_NM);
        ScriptUtils::declareConstant(vm, "_PBSM_EMISSIVE", Ogre::PBSM_EMISSIVE);
        ScriptUtils::declareConstant(vm, "_PBSM_REFLECTION", Ogre::PBSM_REFLECTION);

        ScriptUtils::declareConstant(vm, "_PBS_WORKFLOW_SPECULAROGRE", Ogre::HlmsPbsDatablock::Workflows::SpecularWorkflow);
        ScriptUtils::declareConstant(vm, "_PBS_WORKFLOW_SPECULARFRESNEL", Ogre::HlmsPbsDatablock::Workflows::SpecularAsFresnelWorkflow);
        ScriptUtils::declareConstant(vm, "_PBS_WORKFLOW_METALLIC", Ogre::HlmsPbsDatablock::Workflows::MetallicWorkflow);

        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_NORMAL_NON_PREMUL", Ogre::PBSM_BLEND_NORMAL_NON_PREMUL);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_NORMAL_PREMUL", Ogre::PBSM_BLEND_NORMAL_PREMUL);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_ADD", Ogre::PBSM_BLEND_ADD);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_SUBTRACT", Ogre::PBSM_BLEND_SUBTRACT);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_MULTIPLY", Ogre::PBSM_BLEND_MULTIPLY);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_MULTIPLY2X", Ogre::PBSM_BLEND_MULTIPLY2X);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_SCREEN", Ogre::PBSM_BLEND_SCREEN);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_OVERLAY", Ogre::PBSM_BLEND_OVERLAY);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_LIGHTEN", Ogre::PBSM_BLEND_LIGHTEN);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_DARKEN", Ogre::PBSM_BLEND_DARKEN);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_GRAIN_EXTRACT", Ogre::PBSM_BLEND_GRAIN_EXTRACT);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_GRAIN_MERGE", Ogre::PBSM_BLEND_GRAIN_MERGE);
        ScriptUtils::declareConstant(vm, "_PBSM_BLEND_DIFFERENCE", Ogre::PBSM_BLEND_DIFFERENCE);

        ScriptUtils::declareConstant(vm, "_HLMS_SBF_ONE", Ogre::SBF_ONE);
        ScriptUtils::declareConstant(vm, "_HLMS_SBF_ZERO", Ogre::SBF_ZERO);
        ScriptUtils::declareConstant(vm, "_HLMS_SBF_DEST_COLOUR", Ogre::SBF_DEST_COLOUR);
        ScriptUtils::declareConstant(vm, "_HLMS_SBF_SOURCE_COLOUR", Ogre::SBF_SOURCE_COLOUR);
        ScriptUtils::declareConstant(vm, "_HLMS_SBF_ONE_MINUS_DEST_COLOUR", Ogre::SBF_ONE_MINUS_DEST_COLOUR);
        ScriptUtils::declareConstant(vm, "_HLMS_SBF_ONE_MINUS_SOURCE_COLOUR", Ogre::SBF_ONE_MINUS_SOURCE_COLOUR);
        ScriptUtils::declareConstant(vm, "_HLMS_SBF_DEST_ALPHA", Ogre::SBF_DEST_ALPHA);
        ScriptUtils::declareConstant(vm, "_HLMS_SBF_SOURCE_ALPHA", Ogre::SBF_SOURCE_ALPHA);
        ScriptUtils::declareConstant(vm, "_HLMS_SBF_ONE_MINUS_DEST_ALPHA", Ogre::SBF_ONE_MINUS_DEST_ALPHA);
        ScriptUtils::declareConstant(vm, "_HLMS_SBF_ONE_MINUS_SOURCE_ALPHA", Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

        ScriptUtils::declareConstant(vm, "_HLMS_SBO_ADD", Ogre::SBO_ADD);
        ScriptUtils::declareConstant(vm, "_HLMS_SBO_SUBTRACT", Ogre::SBO_SUBTRACT);
        ScriptUtils::declareConstant(vm, "_HLMS_SBO_REVERSE_SUBTRACT", Ogre::SBO_REVERSE_SUBTRACT);
        ScriptUtils::declareConstant(vm, "_HLMS_SBO_MIN", Ogre::SBO_MIN);
        ScriptUtils::declareConstant(vm, "_HLMS_SBO_MAX", Ogre::SBO_MAX);
    }
}
