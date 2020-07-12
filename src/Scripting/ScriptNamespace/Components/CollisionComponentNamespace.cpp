#include "CollisionComponentNamespace.h"

#include "Logger/Log.h"
#include "World/Entity/Logic/CollisionComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/MeshClass.h"
#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"
#include "Scripting/ScriptNamespace/Classes/PhysicsClasses/PhysicsObjectUserData.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    SQInteger CollisionComponentNamespace::add(HSQUIRRELVM vm){
        SQInteger size = sq_gettop(vm);

        eId id;
        PhysicsTypes::CollisionObjectPtr a = 0;
        PhysicsTypes::CollisionObjectPtr b = 0;
        if(size == 3){
            SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -2, &id));
            SCRIPT_CHECK_RESULT(PhysicsObjectUserData::getPointerFromUserData(vm, -1, &a));
        }else if(size == 4){
            SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -3, &id));

            SCRIPT_CHECK_RESULT(PhysicsObjectUserData::getPointerFromUserData(vm, -1, &a));
            SCRIPT_CHECK_RESULT(PhysicsObjectUserData::getPointerFromUserData(vm, -2, &b));
        }else{
            return sq_throwerror(vm, "Incorrect number of arguments");
        }

        CollisionComponentLogic::add(id, a, b);

        return 0;
    }

    SQInteger CollisionComponentNamespace::remove(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -1, &id));

        CollisionComponentLogic::remove(id);

        return 0;
    }

    SQInteger CollisionComponentNamespace::getObject(HSQUIRRELVM vm){
        eId id;
        SQBool targetBody;
        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -2, &id));

        sq_getbool(vm, -1, &targetBody);

        PhysicsTypes::CollisionObjectPtr obj = 0;
        if(!CollisionComponentLogic::getBody(id, targetBody, &obj)) return sq_throwerror(vm, "Error reading object from entity.");

        CollisionObjectType::CollisionObjectType type = CollisionWorld::getObjectType(obj);
        PhysicsObjectUserData::collisionObjectFromPointer(vm, obj, type == CollisionObjectType::RECEIVER);

        return 1;
    }

    void CollisionComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("collision"), -1);
        sq_newtableex(vm, 3);

        ScriptUtils::addFunction(vm, add, "add", -3, ".xuu");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".x");
        ScriptUtils::addFunction(vm, getObject, "getObject", 3, ".xb");

        sq_newslot(vm, -3, false);
    }
}
