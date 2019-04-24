#include "ComponentNamespace.h"

#include "Components/MeshComponentNamespace.h"
#include "Components/ScriptComponentNamespace.h"

namespace AV{
    void ComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        MeshComponentNamespace meshComponent;
        meshComponent.setupNamespace(vm);
        
        ScriptComponentNamespace scriptComponent;
        scriptComponent.setupNamespace(vm);
    }
}
