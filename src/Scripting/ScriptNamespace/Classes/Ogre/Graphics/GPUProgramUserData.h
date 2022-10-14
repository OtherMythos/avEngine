#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    class GPUProgramUserData{
    public:
        GPUProgramUserData() = delete;
        ~GPUProgramUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void GPUProgramToUserData(HSQUIRRELVM vm, Ogre::HighLevelGpuProgramPtr program);

        static UserDataGetResult readGPUProgramFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::HighLevelGpuProgramPtr* outProg);

    private:
        static SQObject GPUProgramDelegateTableObject;

        static SQInteger getType(HSQUIRRELVM vm);
    };
}
