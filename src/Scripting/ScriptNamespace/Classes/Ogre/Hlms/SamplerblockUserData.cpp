#include "SamplerblockUserData.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsDatablock.h"
#include "DatablockUnlitDelegate.h"
#include "DatablockPbsDelegate.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    SQObject SamplerblockUserData::samplerblockDelegateTableObject;

    SQInteger SamplerblockUserData::blockReleaseHook(SQUserPointer p, SQInteger size){

        const Ogre::HlmsSamplerblock** samplerblock = static_cast<const Ogre::HlmsSamplerblock**>(p);
        Ogre::Root::getSingletonPtr()->getHlmsManager()->destroySamplerblock(*samplerblock);

        return 0;
    }

    void SamplerblockUserData::SamplerblockPtrToUserData(HSQUIRRELVM vm, const Ogre::HlmsSamplerblock* samplerblock){
        void* pointer = (void*)sq_newuserdata(vm, sizeof(const Ogre::HlmsSamplerblock*));
        const Ogre::HlmsSamplerblock** p = new (pointer) const Ogre::HlmsSamplerblock*;
        *p = samplerblock;

        sq_setreleasehook(vm, -1, blockReleaseHook);
        sq_settypetag(vm, -1, samplerblockTypeTag);

        //Was pushing null. That should be re-enabled when I actually have some need for it.
        //sq_pushobject(vm, SamplerblockDelegateTableObject);
        //sq_setdelegate(vm, -2); //This pops the pushed table
    }

    UserDataGetResult SamplerblockUserData::getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, const Ogre::HlmsSamplerblock** outPtr){
        SQUserPointer pointer, typeTag;
        sq_getuserdata(vm, stackInx, &pointer, &typeTag);
        if(typeTag != samplerblockTypeTag){
            *outPtr = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        const Ogre::HlmsSamplerblock** p = (const Ogre::HlmsSamplerblock**)pointer;
        *outPtr = *p;

        return USER_DATA_GET_SUCCESS;
    }

    void SamplerblockUserData::setupDelegateTable(HSQUIRRELVM vm){

        //sq_newtableex(vm, 4);
        sq_newtable(vm);

        //ScriptUtils::addFunction(vm, setSamplerblockValue, "setValue");

        sq_resetobject(&samplerblockDelegateTableObject);
        sq_getstackobj(vm, -1, &samplerblockDelegateTableObject);
        sq_addref(vm, &samplerblockDelegateTableObject);
        sq_pop(vm, 1);
    }
}
