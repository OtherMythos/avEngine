#include "HlmsNamespace.h"

#include "Logger/Log.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/DatablockUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/MacroblockUserData.h"

#include "Classes/Ogre/MacroblockUserData.h"

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

        sq_getstring(vm, 2, &dbName);

        if(top >= 3){
            SQObjectType t = sq_gettype(vm, 3);
            if(t != OT_NULL){ //Must be a user data if it's not null.
                assert(t == OT_USERDATA);
                bool success = MacroblockUserData::getPtrFromUserData(vm, 3, &targetMacroblock);
                if(!success){
                    return sq_throwerror(vm, "Incorrect object passed as macroblock");
                }
            }
        }
        Ogre::HlmsParamVec vec;
        if(top >= 4){
            SQObjectType t = sq_gettype(vm, 4);
            if(t != OT_NULL){
                assert(t == OT_TABLE);
                _populateDatablockConstructionInfo(vm, 4, vec);
            }
        }

        Ogre::HlmsDatablock* newBlock = 0;
        {
            using namespace Ogre;
            Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(type);
            HlmsType* unlitHlms = dynamic_cast<HlmsType*>(hlms);

            Ogre::HlmsMacroblock macroblock;
            if(targetMacroblock) macroblock = *targetMacroblock;

            try{
                newBlock = unlitHlms->createDatablock(Ogre::IdString(dbName), dbName, macroblock, Ogre::HlmsBlendblock(), vec);
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
            if(!DatablockUserData::getPtrFromUserData(vm, -1, &db)){
                return sq_throwerror(vm, "Incorrect object passed as datablock.");
            }
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

    void HlmsNamespace::setupNamespace(HSQUIRRELVM vm){
        //pbs
        {
            sq_pushstring(vm, _SC("pbs"), -1);
            sq_newtableex(vm, 2);

            /**
            createDatablock(string datablockName, Macroblock block = null, table constructionParams = {});
            */
            ScriptUtils::addFunction(vm, PBSCreateDatablock, "createDatablock", -2, ".s u|o t|o");

            sq_newslot(vm,-3,SQFalse);
        }

        //unlit
        {
            sq_pushstring(vm, _SC("unlit"), -1);
            sq_newtableex(vm, 2);

            ScriptUtils::addFunction(vm, UnlitCreateDatablock, "createDatablock", -2, ".s u|o t|o");//string, macroblock, construction info

            sq_newslot(vm,-3,SQFalse);
        }

        ScriptUtils::addFunction(vm, getDatablock, "getDatablock", 2, ".s");
        ScriptUtils::addFunction(vm, destroyDatablock, "destroyDatablock", 2, ". s|u");

        ScriptUtils::addFunction(vm, getMacroblock, "getMacroblock", 2, ".t");
    }
}
