#pragma once

#include <squirrel.h>
#include "OgrePrerequisites.h"

namespace AV{
    class MacroblockUserData{
    public:
        MacroblockUserData() = delete;

        static void MacroblockPtrToUserData(HSQUIRRELVM vm, const Ogre::HlmsMacroblock* macroblock);
        static bool getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, const Ogre::HlmsMacroblock** outPtr);

        //static SQInteger equalsDatablock(HSQUIRRELVM vm);
        static SQInteger setMacroblockValue(HSQUIRRELVM vm);

        static void setupDelegateTable(HSQUIRRELVM vm);

    private:
        static SQInteger blockReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject macroblockDelegateTableObject;
    };
}
