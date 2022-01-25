#pragma once

#include <squirrel.h>
#include "OgrePrerequisites.h"

namespace AV{
    class BlendblockUserData{
    public:
        BlendblockUserData() = delete;

        static void BlendblockPtrToUserData(HSQUIRRELVM vm, const Ogre::HlmsBlendblock* Blendblock);
        static bool getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, const Ogre::HlmsBlendblock** outPtr);

        //static SQInteger equalsDatablock(HSQUIRRELVM vm);
        static SQInteger setBlendblockValue(HSQUIRRELVM vm);

        static void setupDelegateTable(HSQUIRRELVM vm);

    private:
        static SQInteger blockReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject BlendblockDelegateTableObject;
    };
}
