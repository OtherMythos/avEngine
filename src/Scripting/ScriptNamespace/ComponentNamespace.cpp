#include "ComponentNamespace.h"

#include "Components/MeshComponentNamespace.h"
#include "Components/ScriptComponentNamespace.h"
#include "Components/RigidBodyComponentNamespace.h"
#include "Components/CollisionComponentNamespace.h"
#include "Components/NavigationComponentNamespace.h"
#include "Components/UserComponentNamespace.h"
#include "Components/LifetimeComponentNamespace.h"
#include "Components/SceneNodeComponentNamespace.h"
#include "Components/AnimationComponentNamespace.h"

namespace AV{
    void ComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        MeshComponentNamespace meshComponent;
        meshComponent.setupNamespace(vm);

        ScriptComponentNamespace scriptComponent;
        scriptComponent.setupNamespace(vm);

        RigidBodyComponentNamespace rigidBodyComponent;
        rigidBodyComponent.setupNamespace(vm);

        CollisionComponentNamespace collisionNamespace;
        collisionNamespace.setupNamespace(vm);

        NavigationComponentNamespace navigationNamespace;
        navigationNamespace.setupNamespace(vm);

        UserComponentNamespace userNamespace;
        userNamespace.setupNamespace(vm);

        LifetimeComponentNamespace lifetimeNamespace;
        lifetimeNamespace.setupNamespace(vm);

        SceneNodeComponentNamespace sceneNodeComponentNamespace;
        sceneNodeComponentNamespace.setupNamespace(vm);

        AnimationComponentNamespace animationComponentNamespace;
        animationComponentNamespace.setupNamespace(vm);
    }
}
