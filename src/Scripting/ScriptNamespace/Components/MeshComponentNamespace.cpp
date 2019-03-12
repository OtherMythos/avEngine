#include "MeshComponentNamespace.h"

#include "Logger/Log.h"

namespace AV{

    SQInteger MeshComponentNamespace::add(HSQUIRRELVM v){
        AV_INFO("Mesh component");

        return 0;
    }

    void MeshComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("mesh"), -1);
        sq_newtable(vm);

        _addFunction(vm, add, "add");

        sq_newslot(vm, -3, false);
    }
}
