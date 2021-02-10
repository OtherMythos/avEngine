#include "AnimationComponentNamespace.h"

#include "Logger/Log.h"
#include "World/Entity/Logic/AnimationComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"
#include "Animation/AnimationData.h"
#include "Scripting/ScriptNamespace/Classes/Animation/AnimationInstanceUserData.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    SQInteger AnimationComponentNamespace::add(HSQUIRRELVM vm){
        SQInteger size = sq_gettop(vm);

        eId id;
        SequenceAnimationPtr a = 0;
        SequenceAnimationPtr b = 0;

        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, 2, &id));
        //If null then continue as normal. Adding the component without an animation can be useful.
        if(sq_gettype(vm, 3) != OT_NULL){
            SCRIPT_CHECK_RESULT(AnimationInstanceUserData::readAnimationPtrFromUserData(vm, 3, &a));
        }
        if(size == 3) {}
        else if(size == 4){
            if(sq_gettype(vm, 4) != OT_NULL) SCRIPT_CHECK_RESULT(AnimationInstanceUserData::readAnimationPtrFromUserData(vm, 4, &b));
        }else{
            return sq_throwerror(vm, "Incorrect number of arguments");
        }

        AnimationComponentLogic::add(id, a, b, true, size >= 4);

        return 0;
    }

    SQInteger AnimationComponentNamespace::remove(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -1, &id));

        AnimationComponentLogic::remove(id);

        return 0;
    }

    SQInteger AnimationComponentNamespace::get(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, 2, &id));

        SQInteger target;
        sq_getinteger(vm, 3, &target);
        if(target < 0 || target > 1) return sq_throwerror(vm, "Target must be in range of 0 and 1");

        SequenceAnimationPtr ptr = AnimationComponentLogic::getAnimation(id, static_cast<uint8>(target));
        AnimationInstanceUserData::animationPtrToUserData(vm, ptr);

        return 1;
    }

    SQInteger AnimationComponentNamespace::set(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, 2, &id));

        SQInteger target;
        sq_getinteger(vm, 3, &target);
        if(target < 0 || target > 1) return sq_throwerror(vm, "Target must be in range of 0 and 1");

        SequenceAnimationPtr a = 0;
        SCRIPT_CHECK_RESULT(AnimationInstanceUserData::readAnimationPtrFromUserData(vm, 4, &a));

        bool result = AnimationComponentLogic::setAnimation(id, static_cast<uint8>(target), a);
        if(!result) return sq_throwerror(vm, "Unable to set pointer to animation.");

        return 0;
    }

    void AnimationComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("animation"), -1);
        sq_newtableex(vm, 3);

        ScriptUtils::addFunction(vm, add, "add", -3, ".x u|o u|o");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".x");

        ScriptUtils::addFunction(vm, get, "get", 3, ".xi");
        ScriptUtils::addFunction(vm, set, "set", 4, ".xiu");

        sq_newslot(vm, -3, false);
    }
}
