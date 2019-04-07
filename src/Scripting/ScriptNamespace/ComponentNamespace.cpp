#include "ComponentNamespace.h"

#include "Components/MeshComponentNamespace.h"

namespace AV{
    void ComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        MeshComponentNamespace meshComponent;
        meshComponent.setupNamespace(vm);
    }
}
