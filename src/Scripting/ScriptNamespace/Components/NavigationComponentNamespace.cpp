#include "NavigationComponentNamespace.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "World/Entity/Logic/NavigationComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/Entity/EntityUserData.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"

namespace AV{

    SQInteger NavigationComponentNamespace::add(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &id));

        NavigationComponentLogic::add(id);

        return 0;
    }

    SQInteger NavigationComponentNamespace::remove(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &id));

        NavigationComponentLogic::remove(id);

        return 0;
    }

    SQInteger NavigationComponentNamespace::navigateTo(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, 2, &id));

        SlotPosition targetPos;
        SCRIPT_CHECK_RESULT(SlotPositionClass::getSlotFromInstance(vm, 3, &targetPos));

        SQFloat targetSpeed = 1.0f;

        SQInteger nargs = sq_gettop(vm);
        if(nargs == 4){
            sq_getfloat(vm, 4, &targetSpeed);
            if(targetSpeed <= 0) sq_throwerror(vm, "speed must be greater than 0.");
        }

        NavigationComponentLogic::navigateTo(id, targetPos, static_cast<float>(targetSpeed));

        return 0;
    }

    void NavigationComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("navigation"), -1);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, add, "add", 2, ".u");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".u");
        ScriptUtils::addFunction(vm, navigateTo, "navigateTo", -3, ".uun");

        sq_newslot(vm, -3, false);
    }
}
