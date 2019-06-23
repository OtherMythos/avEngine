#include "MeshNamespace.h"

#include "World/Support/OgreMeshManager.h"
#include "Scripting/ScriptNamespace/Classes/MeshClass.h"
#include "System/BaseSingleton.h"

namespace AV{

    SQInteger MeshNamespace::createMesh(HSQUIRRELVM vm){
        OgreMeshManager::OgreMeshPtr mesh = BaseSingleton::getOgreMeshManager()->createMesh("ogrehead2.mesh");

        MeshClass::MeshPtrToInstance(vm, mesh);

        return 1;
    }

    void MeshNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, createMesh, "create", 2, ".s");
    }
}
