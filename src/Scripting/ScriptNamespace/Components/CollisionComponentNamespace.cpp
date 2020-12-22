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
        PhysicsTypes::CollisionObjectPtr c = 0;

        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, 2, &id));
        SCRIPT_CHECK_RESULT(PhysicsObjectUserData::getPointerFromUserData(vm, 3, &a));
        if(size == 3) {}
        else if(size == 4){
            SCRIPT_CHECK_RESULT(PhysicsObjectUserData::getPointerFromUserData(vm, 4, &b));
        }else if(size == 5){
            SCRIPT_CHECK_RESULT(PhysicsObjectUserData::getPointerFromUserData(vm, 4, &b));
            SCRIPT_CHECK_RESULT(PhysicsObjectUserData::getPointerFromUserData(vm, 5, &c));
        }else{
            return sq_throwerror(vm, "Incorrect number of arguments");
        }

        //The last two are whether the component contains a populated pointer.
        //I assume the first one is always populated, if the stack size is correct the second one will also be populated.
        CollisionComponentLogic::add(id, a, b, c, true, size >= 4, size >= 5);

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

        ScriptUtils::addFunction(vm, add, "add", -3, ".xuuu");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".x");
        ScriptUtils::addFunction(vm, getObject, "getObject", 3, ".xb");

        sq_newslot(vm, -3, false);
    }
}
