#include "BlendblockUserData.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsDatablock.h"
#include "DatablockUnlitDelegate.h"
#include "DatablockPbsDelegate.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    SQObject BlendblockUserData::BlendblockDelegateTableObject;

    SQInteger BlendblockUserData::blockReleaseHook(SQUserPointer p, SQInteger size){

        const Ogre::HlmsBlendblock** Blendblock = static_cast<const Ogre::HlmsBlendblock**>(p);
        Ogre::Root::getSingletonPtr()->getHlmsManager()->destroyBlendblock(*Blendblock);

        return 0;
    }

    void BlendblockUserData::BlendblockPtrToUserData(HSQUIRRELVM vm, const Ogre::HlmsBlendblock* Blendblock){
        void* pointer = (void*)sq_newuserdata(vm, sizeof(const Ogre::HlmsBlendblock*));
        const Ogre::HlmsBlendblock** p = new (pointer) const Ogre::HlmsBlendblock*;
        *p = Blendblock;

        sq_setreleasehook(vm, -1, blockReleaseHook);
        sq_settypetag(vm, -1, blendblockTypeTag);

        //Was pushing null. That should be re-enabled when I actually have some need for it.
        //sq_pushobject(vm, BlendblockDelegateTableObject);
        //sq_setdelegate(vm, -2); //This pops the pushed table
    }

    UserDataGetResult BlendblockUserData::getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, const Ogre::HlmsBlendblock** outPtr){
        SQUserPointer pointer, typeTag;
        sq_getuserdata(vm, stackInx, &pointer, &typeTag);
        if(typeTag != blendblockTypeTag){
            *outPtr = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        const Ogre::HlmsBlendblock** p = (const Ogre::HlmsBlendblock**)pointer;
        *outPtr = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger BlendblockUserData::setBlendblockValue(HSQUIRRELVM vm){
        /*Ogre::HlmsBlendblock* Blendblock;
        SCRIPT_ASSERT_RESULT(getPtrFromUserData(vm, 1, &Blendblock));

        SQInteger type, value;
        sq_getinteger(vm, 2, &type);
        sq_getinteger(vm, 3, &value);*/

        return 0;
    }

    void BlendblockUserData::setupDelegateTable(HSQUIRRELVM vm){

        //sq_newtableex(vm, 4);
        sq_newtable(vm);

        //ScriptUtils::addFunction(vm, setBlendblockValue, "setValue");

        sq_resetobject(&BlendblockDelegateTableObject);
        sq_getstackobj(vm, -1, &BlendblockDelegateTableObject);
        sq_addref(vm, &BlendblockDelegateTableObject);
        sq_pop(vm, 1);
    }
}
