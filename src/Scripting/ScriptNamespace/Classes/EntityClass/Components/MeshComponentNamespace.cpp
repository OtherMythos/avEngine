#include "MeshComponentNamespace.h"

#include "Logger/Log.h"
#include "World/Entity/Logic/OgreMeshComponentLogic.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    SQInteger MeshComponentNamespace::add(HSQUIRRELVM v){
        const SQChar *meshName;
        sq_getstring(v, -1, &meshName);

        sq_pushstring(v, _SC("_entity"), 0);
        sq_get(v, -3);

        ScriptUtils::_debugStack(v);

        OgreMeshComponentLogic::add(_getEID(v, -2), Ogre::String(meshName));

        return 0;
    }

    SQInteger MeshComponentNamespace::remove(HSQUIRRELVM v){
        eId id = _getEID(v, -1);

        OgreMeshComponentLogic::remove(id);

        return 0;
    }

    void MeshComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("mesh"), -1);
        sq_newtable(vm);

        _addFunction(vm, add, "add", 2, ".s");
        _addFunction(vm, remove, "remove", 2, ".u");

        sq_newslot(vm, -3, false);
    }
}
