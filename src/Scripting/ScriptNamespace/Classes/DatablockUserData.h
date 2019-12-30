#pragma once

#include <squirrel.h>
#include "OgrePrerequisites.h"

namespace AV{
    class DatablockUserData{
    public:
        DatablockUserData() { }

        static void DatablockPtrToUserData(HSQUIRRELVM vm, Ogre::HlmsDatablock* db);
        static Ogre::HlmsDatablock* getPtrFromUserData(HSQUIRRELVM vm);

        static void setupDelegateTable(HSQUIRRELVM vm);

    private:
        static SQInteger blockReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger setDiffuse(HSQUIRRELVM vm);

        static SQObject delegateTableObject;
    };
}
