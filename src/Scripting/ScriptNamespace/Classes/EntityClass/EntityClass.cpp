#include "EntityClass.h"

#include "Components/MeshComponentNamespace.h"

namespace AV{
    void EntityClass::setupClass(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("entity"), -1);
        sq_newclass(vm, 0);

        sq_pushstring(vm, _SC("x"), -1);
        sq_pushfloat(vm, 0);
        sq_newslot(vm, -3, false);

        // MeshComponentNamespace meshComponent;
        // meshComponent.setupNamespace(vm);

        sq_newslot(vm, -3 , false);
    }
}
