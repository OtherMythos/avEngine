#include "MacroblockUserData.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsDatablock.h"
#include "DatablockUnlitDelegate.h"
#include "DatablockPbsDelegate.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    SQObject MacroblockUserData::macroblockDelegateTableObject;

    SQInteger MacroblockUserData::blockReleaseHook(SQUserPointer p, SQInteger size){

        const Ogre::HlmsMacroblock** macroblock = static_cast<const Ogre::HlmsMacroblock**>(p);
        Ogre::Root::getSingletonPtr()->getHlmsManager()->destroyMacroblock(*macroblock);

        return 0;
    }

    void MacroblockUserData::MacroblockPtrToUserData(HSQUIRRELVM vm, const Ogre::HlmsMacroblock* macroblock){
        void* pointer = (void*)sq_newuserdata(vm, sizeof(const Ogre::HlmsMacroblock*));
        const Ogre::HlmsMacroblock** p = new (pointer) const Ogre::HlmsMacroblock*;
        *p = macroblock;

        sq_setreleasehook(vm, -1, blockReleaseHook);
        sq_settypetag(vm, -1, macroblockTypeTag);

        //Was pushing null. That should be re-enabled when I actually have some need for it.
        //sq_pushobject(vm, macroblockDelegateTableObject);
        //sq_setdelegate(vm, -2); //This pops the pushed table
    }

    bool MacroblockUserData::getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, const Ogre::HlmsMacroblock** outPtr){
        SQUserPointer pointer, typeTag;
        sq_getuserdata(vm, stackInx, &pointer, &typeTag);
        if(typeTag != macroblockTypeTag){
            *outPtr = 0;
            return false;
        }

        const Ogre::HlmsMacroblock** p = (const Ogre::HlmsMacroblock**)pointer;
        *outPtr = *p;

        return true;
    }

    void MacroblockUserData::setupDelegateTable(HSQUIRRELVM vm){

        //sq_newtableex(vm, 4);
        sq_newtable(vm);

        sq_resetobject(&macroblockDelegateTableObject);
        sq_getstackobj(vm, -1, &macroblockDelegateTableObject);
        sq_addref(vm, &macroblockDelegateTableObject);
        sq_pop(vm, 1);
    }
}
