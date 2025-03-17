#pragma once

#include <squirrel.h>
#include "OgrePrerequisites.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class SamplerblockUserData{
    public:
        SamplerblockUserData() = delete;

        static void SamplerblockPtrToUserData(HSQUIRRELVM vm, const Ogre::HlmsSamplerblock* samplerblock);
        static UserDataGetResult getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, const Ogre::HlmsSamplerblock** outPtr);

        static void setupDelegateTable(HSQUIRRELVM vm);

    private:
        static SQInteger blockReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject samplerblockDelegateTableObject;
    };
}
