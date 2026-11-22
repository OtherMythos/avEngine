#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "OgrePrerequisites.h"

namespace AV{
    class DatablockUserData{
    public:
        DatablockUserData() = delete;

        static void DatablockPtrToUserData(HSQUIRRELVM vm, Ogre::HlmsDatablock* db);
        static UserDataGetResult getPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::HlmsDatablock** outPtr);

        static SQInteger equalsDatablock(HSQUIRRELVM vm);
        static SQInteger cloneDatablock(HSQUIRRELVM vm);
        static SQInteger getName(HSQUIRRELVM vm);
        static SQInteger setCustomPieceFile(HSQUIRRELVM vm);
        static SQInteger setCustomPieceCode(HSQUIRRELVM vm);
        static SQInteger getCustomPieceFile(HSQUIRRELVM vm);
        static SQInteger setCustomProperties(HSQUIRRELVM vm);
        static SQInteger getCustomProperties(HSQUIRRELVM vm);

        /// Register the functions shared by every datablock type into the table on top of the stack.
        static void setupSharedFunctions(HSQUIRRELVM vm);

        static void setupDelegateTable(HSQUIRRELVM vm);

    private:
        static SQInteger blockReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject pbsDelegateTableObject;
        static SQObject unlitDelegateTableObject;
    };
}
