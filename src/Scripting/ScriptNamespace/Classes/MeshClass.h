#pragma once

#include <squirrel.h>

#include "World/Support/OgreMeshManager.h"
#include "Scripting/ScriptDataPacker.h"

namespace AV{

    /**
    A class to expose meshes to squirrel.
    */
    class MeshClass{
    public:
        MeshClass() { };

        static void setupClass(HSQUIRRELVM vm);

        static void MeshPtrToInstance(HSQUIRRELVM vm, OgreMeshManager::OgreMeshPtr mesh);
        static OgreMeshManager::OgreMeshPtr instanceToMeshPtr(HSQUIRRELVM vm, SQInteger index);

    private:
        static ScriptDataPacker<OgreMeshManager::OgreMeshPtr> mMeshData;
        static SQObject classObject;

        static SQInteger setMeshPosition(HSQUIRRELVM vm);
        static SQInteger setScale(HSQUIRRELVM vm);
        static SQInteger setOrientation(HSQUIRRELVM vm);

        static SQInteger sqMeshReleaseHook(SQUserPointer p, SQInteger size);
    };
}
