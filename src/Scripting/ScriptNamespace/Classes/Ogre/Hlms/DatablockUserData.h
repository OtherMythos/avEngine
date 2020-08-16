#pragma once

#include <squirrel.h>
#include "OgrePrerequisites.h"

namespace AV{
    class DatablockUserData{
    public:
        DatablockUserData() = delete;

        static void DatablockPtrToUserData(HSQUIRRELVM vm, Ogre::HlmsDatablock* db);
        static bool getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::HlmsDatablock** outPtr);

        static SQInteger equalsDatablock(HSQUIRRELVM vm);

        static void setupDelegateTable(HSQUIRRELVM vm);

    private:
        static SQInteger blockReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject pbsDelegateTableObject;
        static SQObject unlitDelegateTableObject;
    };
}
