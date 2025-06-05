#include "AnimationInstanceUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "System/BaseSingleton.h"
#include "Animation/AnimationManager.h"

namespace AV{

    SQObject AnimationInstanceUserData::animDelegateTable;

    void AnimationInstanceUserData::animationPtrToUserData(HSQUIRRELVM vm, SequenceAnimationPtr ptr){
        SequenceAnimationPtr* pointer = (SequenceAnimationPtr*)sq_newuserdata(vm, sizeof(SequenceAnimationPtr));
        new (pointer)SequenceAnimationPtr(ptr);

        sq_settypetag(vm, -1, AnimationInstanceTypeTag);
        sq_setreleasehook(vm, -1, AnimationObjectReleaseHook);
        sq_pushobject(vm, animDelegateTable);
        sq_setdelegate(vm, -2); //This pops the pushed table
    }

    UserDataGetResult AnimationInstanceUserData::readAnimationPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, SequenceAnimationPtr* outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != AnimationInstanceTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        SequenceAnimationPtr* p = static_cast<SequenceAnimationPtr*>(pointer);
        *outObject = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger AnimationInstanceUserData::AnimationObjectReleaseHook(SQUserPointer p, SQInteger size){
        SequenceAnimationPtr* ptr = static_cast<SequenceAnimationPtr*>(p);
        ptr->reset();

        return 0;
    }

    SQInteger AnimationInstanceUserData::isRunning(HSQUIRRELVM vm){
        SequenceAnimationPtr p;
        SCRIPT_ASSERT_RESULT(readAnimationPtrFromUserData(vm, 1, &p));

        bool result = BaseSingleton::getAnimationManager()->isAnimRunning(p);
        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger AnimationInstanceUserData::getTime(HSQUIRRELVM vm){
        SequenceAnimationPtr p;
        SCRIPT_ASSERT_RESULT(readAnimationPtrFromUserData(vm, 1, &p));

        uint16 result = BaseSingleton::getAnimationManager()->getAnimTime(p);
        sq_pushinteger(vm, static_cast<SQInteger>(result) );

        return 1;
    }

    SQInteger AnimationInstanceUserData::setTime(HSQUIRRELVM vm){
        SequenceAnimationPtr p;
        SCRIPT_ASSERT_RESULT(readAnimationPtrFromUserData(vm, 1, &p));

        SQInteger animTime;
        sq_getinteger(vm, 2, &animTime);
        uint16 timeVal = static_cast<uint16>(animTime);

        SQBool update = false;
        if(sq_gettop(vm) >= 3){
            sq_getbool(vm, 3, &update);
        }

        BaseSingleton::getAnimationManager()->setAnimTime(p, timeVal, update);

        return 0;
    }

    SQInteger AnimationInstanceUserData::setRunning(HSQUIRRELVM vm){
        SequenceAnimationPtr p;
        SCRIPT_ASSERT_RESULT(readAnimationPtrFromUserData(vm, 1, &p));

        SQBool shouldRun;
        sq_getbool(vm, 2, &shouldRun);

        BaseSingleton::getAnimationManager()->setAnimationRunning(p, shouldRun);

        return 0;
    }

    void AnimationInstanceUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, isRunning, "isRunning");
        ScriptUtils::addFunction(vm, getTime, "getTime");
        ScriptUtils::addFunction(vm, setTime, "setTime", -2, ".ib");

        ScriptUtils::addFunction(vm, setRunning, "setRunning", 2, ".b");

        sq_resetobject(&animDelegateTable);
        sq_getstackobj(vm, -1, &animDelegateTable);
        sq_addref(vm, &animDelegateTable);
        sq_pop(vm, 1);

    }

}
