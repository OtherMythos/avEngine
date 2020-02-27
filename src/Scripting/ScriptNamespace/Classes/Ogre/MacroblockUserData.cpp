#include "MacroblockUserData.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsDatablock.h"
#include "DatablockUnlitDelegate.h"
#include "DatablockPbsDelegate.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

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

        sq_pushobject(vm, macroblockDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
    }

    /*SQInteger MacroblockUserData::equalsDatablock(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db1 = getPtrFromUserData(vm, -1);
        Ogre::HlmsDatablock* db2 = getPtrFromUserData(vm, -2);

        sq_pushbool(vm, db1 == db2);

        return 1;
    }*/

    const Ogre::HlmsMacroblock* MacroblockUserData::getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx){
        SQUserPointer pointer;
        sq_getuserdata(vm, stackInx, &pointer, NULL);

        const Ogre::HlmsMacroblock** p = (const Ogre::HlmsMacroblock**)pointer;

        return *p;
    }

    void MacroblockUserData::setupDelegateTable(HSQUIRRELVM vm){

        sq_newtableex(vm, 4);

        sq_resetobject(&macroblockDelegateTableObject);
        sq_getstackobj(vm, -1, &macroblockDelegateTableObject);
        sq_addref(vm, &macroblockDelegateTableObject);
        sq_pop(vm, 1);
    }
}
