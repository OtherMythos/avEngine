#include "RigidBodyComponentNamespace.h"

#include "World/Entity/Logic/RigidBodyComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"
#include "Scripting/ScriptNamespace/Classes/PhysicsClasses/PhysicsRigidBodyClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    SQInteger RigidBodyComponentNamespace::add(HSQUIRRELVM vm){
        eId id = EntityClass::getEID(vm, -2);

        DynamicsWorld::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);
        RigidBodyComponentLogic::add(id, body);

        return 0;
    }

    SQInteger RigidBodyComponentNamespace::remove(HSQUIRRELVM vm){
        eId id = EntityClass::getEID(vm, -1);

        RigidBodyComponentLogic::remove(id);

        return 0;
    }

    void RigidBodyComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("rigidBody"), -1);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, add, "add", 3, ".xx");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".x");

        sq_newslot(vm, -3, false);
    }
}
