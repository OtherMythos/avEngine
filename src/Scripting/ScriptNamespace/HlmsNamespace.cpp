#include "HlmsNamespace.h"

#include "Logger/Log.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/DatablockUserData.h"

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
        sq_pushstring(vm, _SC("pbs"), -1);
        sq_newtableex(vm, 2);

        ScriptUtils::addFunction(vm, PBSCreateDatablock, "createDatablock", 2, ".s");

        sq_newslot(vm,-3,SQFalse);


        //unlit
        sq_pushstring(vm, _SC("unlit"), -1);
        sq_newtableex(vm, 2);

        ScriptUtils::addFunction(vm, UnlitCreateDatablock, "createDatablock", 2, ".s");

        sq_newslot(vm,-3,SQFalse);

        ScriptUtils::addFunction(vm, getDatablock, "getDatablock", 2, ".s");
        ScriptUtils::addFunction(vm, destroyDatablock, "destroyDatablock", 2, ".s");
    }
}
