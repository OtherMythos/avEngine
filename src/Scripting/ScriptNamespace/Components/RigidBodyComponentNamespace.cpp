#include "RigidBodyComponentNamespace.h"

#include "World/Entity/Logic/RigidBodyComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/Entity/EntityUserData.h"
#include "Scripting/ScriptNamespace/Classes/PhysicsClasses/PhysicsRigidBodyClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include <map>

namespace AV{

    SQInteger RigidBodyComponentNamespace::add(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -2, &id));

        PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);
        bool result = RigidBodyComponentLogic::add(id, body);

        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger RigidBodyComponentNamespace::remove(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &id));

        RigidBodyComponentLogic::remove(id);

        return 0;
    }

    SQInteger RigidBodyComponentNamespace::getRigidBody(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &id));

        PhysicsTypes::RigidBodyPtr body;
        bool successful = RigidBodyComponentLogic::getBody(id, body);
        if(successful){
            PhysicsRigidBodyClass::createInstanceFromPointer(vm, body);
            return 1;
        }

        return 0;
    }

    void RigidBodyComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("rigidBody"), -1);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, add, "add", 3, ".ux");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".u");
        ScriptUtils::addFunction(vm, getRigidBody, "get", 0, ".");

        sq_newslot(vm, -3, false);
    }
}
