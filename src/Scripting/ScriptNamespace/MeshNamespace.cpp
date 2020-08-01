#include "MeshNamespace.h"

#include "World/Support/OgreMeshManager.h"
#include "Scripting/ScriptNamespace/Classes/MeshClass.h"
#include "System/BaseSingleton.h"

namespace AV{

    SQInteger MeshNamespace::createMesh(HSQUIRRELVM vm){
        const SQChar *meshPath;
        sq_getstring(vm, -1, &meshPath);

        OgreMeshManager::OgreMeshPtr mesh = BaseSingleton::getOgreMeshManager()->createMesh(meshPath);
        if(!mesh){
            std::string s("Error reading mesh with name: ");
            s += meshPath;
            return sq_throwerror(vm, s.c_str());
        }

        MeshClass::MeshPtrToInstance(vm, mesh);

        return 1;
    }

    /**SQNamespace
    @name _mesh
    @desc Functions to create meshes.
    */
    void MeshNamespace::setupNamespace(HSQUIRRELVM vm){

        /**SQFunction
        @name create

        @desc Create a mesh.

        @returns a mesh instance.
        */
        ScriptUtils::addFunction(vm, createMesh, "create", 2, ".s");
    }
}
