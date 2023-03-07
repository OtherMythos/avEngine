#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    class SubMeshUserData{
    public:
        SubMeshUserData() = delete;
        ~SubMeshUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void SubMeshToUserData(HSQUIRRELVM vm, Ogre::SubMesh* program);

        static UserDataGetResult readSubMeshFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::SubMesh** outProg);

    private:
        static SQObject SubMeshDelegateTableObject;

        static SQInteger getMaterialName(HSQUIRRELVM vm);
        static SQInteger SubMeshToString(HSQUIRRELVM vm);
        static SQInteger SubMeshCompare(HSQUIRRELVM vm);
        static SQInteger pushMeshVAO(HSQUIRRELVM vm);
        static SQInteger clearMeshVAO(HSQUIRRELVM vm);

        static SQInteger SubMeshObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
