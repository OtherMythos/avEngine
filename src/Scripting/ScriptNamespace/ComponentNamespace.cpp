#include "ComponentNamespace.h"

#include "Components/MeshComponentNamespace.h"

namespace AV{
    void ComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("_component"), -1);
        sq_newtable(vm);

        MeshComponentNamespace meshComponent;
        meshComponent.setupNamespace(vm);

        sq_newslot(vm, -3 , false);
    }
}
