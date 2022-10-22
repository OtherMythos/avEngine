#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    class MaterialUserData{
    public:
        MaterialUserData() = delete;
        ~MaterialUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void MaterialToUserData(HSQUIRRELVM vm, Ogre::MaterialPtr program);

        static UserDataGetResult readMaterialFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::MaterialPtr* outProg);

    private:
        static SQObject MaterialDelegateTableObject;

        static SQInteger getName(HSQUIRRELVM vm);
        static SQInteger getVertexProgram(HSQUIRRELVM vm);
        static SQInteger getFragmentProgram(HSQUIRRELVM vm);
        static SQInteger getVertexProgramParameters(HSQUIRRELVM vm);
        static SQInteger getFragmentProgramParameters(HSQUIRRELVM vm);
    };
}
