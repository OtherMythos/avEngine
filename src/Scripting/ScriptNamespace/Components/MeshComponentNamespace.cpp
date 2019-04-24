#include "MeshComponentNamespace.h"

#include "Logger/Log.h"
#include "World/Entity/Logic/OgreMeshComponentLogic.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    SQInteger MeshComponentNamespace::add(HSQUIRRELVM v){
        const SQChar *meshName;
        sq_getstring(v, -1, &meshName);

        OgreMeshComponentLogic::add(ScriptUtils::getEID(v, -2), Ogre::String(meshName));

        return 0;
    }

    SQInteger MeshComponentNamespace::remove(HSQUIRRELVM v){
        eId id = ScriptUtils::getEID(v, -1);

        OgreMeshComponentLogic::remove(id);

        return 0;
    }

    void MeshComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("mesh"), -1);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, add, "add", 3, ".xs");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".x");

        sq_newslot(vm, -3, false);
    }
}
