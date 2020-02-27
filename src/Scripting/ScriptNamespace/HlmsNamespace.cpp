#include "HlmsNamespace.h"

#include "Logger/Log.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/DatablockUserData.h"

#include "Classes/Ogre/MacroblockUserData.h"

#include "Ogre.h"
#include "OgreHlms.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsUnlit.h"
#include "OgreHlmsPbsDatablock.h"

namespace AV {
    SQInteger HlmsNamespace::PBSCreateDatablock(HSQUIRRELVM vm){
        const SQChar *dbName;
        sq_getstring(vm, -1, &dbName);

        Ogre::HlmsDatablock* newBlock = 0;
        {
            using namespace Ogre;
            Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_PBS);
            Ogre::HlmsPbs* pbsHlms = dynamic_cast<Ogre::HlmsPbs*>(hlms);

            try{
                newBlock = pbsHlms->createDatablock(Ogre::IdString(dbName), dbName, Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec());
            }catch(Ogre::ItemIdentityException e){
                AV_ERROR("Could not create a new datablock with the name '{}', as one already exists.", dbName);
            }

        }

        if(!newBlock){
            return 0;
        }


        DatablockUserData::DatablockPtrToUserData(vm, (Ogre::HlmsDatablock*)newBlock);

        return 1;
    }

    SQInteger HlmsNamespace::UnlitCreateDatablock(HSQUIRRELVM vm){
        const SQChar *dbName;
        sq_getstring(vm, -1, &dbName);

        Ogre::HlmsDatablock* newBlock = 0;
        {
            using namespace Ogre;
            Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
            Ogre::HlmsUnlit* unlitHlms = dynamic_cast<Ogre::HlmsUnlit*>(hlms);

            try{
                newBlock = unlitHlms->createDatablock(Ogre::IdString(dbName), dbName, Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec());
            }catch(Ogre::ItemIdentityException e){
                AV_ERROR("Could not create a new datablock with the name '{}', as one already exists.", dbName);
            }

        }

        if(!newBlock){
            return 0;
        }

        DatablockUserData::DatablockPtrToUserData(vm, (Ogre::HlmsDatablock*)newBlock);
        return 1;
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
                    block->mPolygonMode = (Ogre::PolygonMode)val;
                }else if(strcmp(k, "cullMode") == 0){
                    block->mCullMode = (Ogre::CullingMode)val;
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
        const SQChar *dbName;
        sq_getstring(vm, -1, &dbName);

        Ogre::HlmsManager* mgr = Ogre::Root::getSingletonPtr()->getHlmsManager();
        Ogre::HlmsDatablock* db = mgr->getDatablockNoDefault(dbName);

        if(!db) return 0; //No actual datablock was found, so there was nothing to delete.

        Ogre::Hlms* hlms = mgr->getHlms( static_cast<Ogre::HlmsTypes>(db->mType));
        assert(hlms);

        if(db->mType == Ogre::HLMS_PBS){
            Ogre::HlmsPbs* pbsHlms = dynamic_cast<Ogre::HlmsPbs*>(hlms);
            //These functions do throw, but by this point it shouldn't have any issues so I don't check them.
            pbsHlms->destroyDatablock(dbName);
        }else if(db->mType == Ogre::HLMS_UNLIT){
            Ogre::HlmsUnlit* unlitHlms = dynamic_cast<Ogre::HlmsUnlit*>(hlms);
            unlitHlms->destroyDatablock(dbName);
        }else assert(false);

        return 0;
    }

    void HlmsNamespace::setupNamespace(HSQUIRRELVM vm){
        //pbs
        {
            sq_pushstring(vm, _SC("pbs"), -1);
            sq_newtableex(vm, 2);

            ScriptUtils::addFunction(vm, PBSCreateDatablock, "createDatablock", 2, ".s");

            sq_newslot(vm,-3,SQFalse);
        }

        //unlit
        {
            sq_pushstring(vm, _SC("unlit"), -1);
            sq_newtableex(vm, 2);

            ScriptUtils::addFunction(vm, UnlitCreateDatablock, "createDatablock", 2, ".s");

            sq_newslot(vm,-3,SQFalse);
        }

        ScriptUtils::addFunction(vm, getDatablock, "getDatablock", 2, ".s");
        ScriptUtils::addFunction(vm, destroyDatablock, "destroyDatablock", 2, ".s");

        ScriptUtils::addFunction(vm, getMacroblock, "getMacroblock", 2, ".t");
    }
}
