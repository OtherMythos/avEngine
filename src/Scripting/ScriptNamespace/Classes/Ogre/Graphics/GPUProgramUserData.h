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

        static void GPUProgramToUserData(HSQUIRRELVM vm, Ogre::GpuProgramPtr program);

        static UserDataGetResult readGPUProgramFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::GpuProgramPtr* outProg);

    private:
        static SQObject GPUProgramDelegateTableObject;

        static SQInteger getType(HSQUIRRELVM vm);
        static SQInteger getNumParameters(HSQUIRRELVM vm);
        static SQInteger getParameterByIdx(HSQUIRRELVM vm);
        static SQInteger setNamedConstant(HSQUIRRELVM vm);
        static SQInteger getNamedConstant(HSQUIRRELVM vm);

        static SQInteger GPUProgramObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
