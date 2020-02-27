#pragma once

#include <squirrel.h>
#include "OgrePrerequisites.h"

namespace AV{
    class MacroblockUserData{
    public:
        MacroblockUserData() = delete;

        static void MacroblockPtrToUserData(HSQUIRRELVM vm, const Ogre::HlmsMacroblock* macroblock);
        static const Ogre::HlmsMacroblock* getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx);

        //static SQInteger equalsDatablock(HSQUIRRELVM vm);

        static void setupDelegateTable(HSQUIRRELVM vm);

    private:
        static SQInteger blockReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject macroblockDelegateTableObject;
    };
}
