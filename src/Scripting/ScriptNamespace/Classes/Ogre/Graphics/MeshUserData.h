#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    class MeshUserData{
    public:
        MeshUserData() = delete;
        ~MeshUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void MeshToUserData(HSQUIRRELVM vm, Ogre::MeshPtr mesh);

        static UserDataGetResult readMeshFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::MeshPtr* outProg);

    private:
        static SQObject MeshDelegateTableObject;

        static SQInteger getName(HSQUIRRELVM vm);
        static SQInteger meshToString(HSQUIRRELVM vm);
        static SQInteger meshCompare(HSQUIRRELVM vm);
        static SQInteger getSubMesh(HSQUIRRELVM vm);
        static SQInteger getNumSubMeshes(HSQUIRRELVM vm);
        static SQInteger createSubMesh(HSQUIRRELVM vm);

        static SQInteger MeshObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
