#include "HlmsNamespace.h"

#include "Logger/Log.h"
#include "Scripting/ScriptNamespace/Classes/DatablockUserData.h"

#include "Ogre.h"
#include "OgreHlms.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsPbsDatablock.h"

namespace AV {
    SQInteger HlmsNamespace::PBSCreateDatablock(HSQUIRRELVM vm){
        const SQChar *dbName;
        sq_getstring(vm, -1, &dbName);

        Ogre::HlmsDatablock* newBlock = 0;
        { //Setup the default datablock for terra.
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

    void HlmsNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("pbs"), -1);
        sq_newtableex(vm, 2);

        _addFunction(vm, PBSCreateDatablock, "createDatablock", 2, ".s");

        sq_newslot(vm,-3,SQFalse);
    }
}
