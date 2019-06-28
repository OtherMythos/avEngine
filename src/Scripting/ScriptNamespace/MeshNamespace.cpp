#include "MeshNamespace.h"

#include "World/Support/OgreMeshManager.h"
#include "Scripting/ScriptNamespace/Classes/MeshClass.h"
#include "System/BaseSingleton.h"

namespace AV{

    SQInteger MeshNamespace::createMesh(HSQUIRRELVM vm){
        const SQChar *meshPath;
        sq_getstring(vm, -1, &meshPath);

        OgreMeshManager::OgreMeshPtr mesh = BaseSingleton::getOgreMeshManager()->createMesh(meshPath);

        MeshClass::MeshPtrToInstance(vm, mesh);

        return 1;
    }

    void MeshNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, createMesh, "create", 2, ".s");
    }
}
