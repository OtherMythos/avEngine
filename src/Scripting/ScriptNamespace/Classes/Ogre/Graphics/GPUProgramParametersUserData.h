#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    class GPUProgramParametersUserData{
    public:
        GPUProgramParametersUserData() = delete;
        ~GPUProgramParametersUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void GPUProgramParametersToUserData(HSQUIRRELVM vm, Ogre::GpuProgramParametersSharedPtr program);

        static UserDataGetResult readGPUProgramParametersFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::GpuProgramParametersSharedPtr* outProg);

    private:
        static SQObject GPUProgramParametersDelegateTableObject;

        static SQInteger setNamedConstant(HSQUIRRELVM vm);
    };
}
