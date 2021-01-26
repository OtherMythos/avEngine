#include "AnimationInfoUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    void AnimationInfoUserData::blockPtrToUserData(HSQUIRRELVM vm, AnimationInfoBlockPtr ptr){
        AnimationInfoBlockPtr* pointer = (AnimationInfoBlockPtr*)sq_newuserdata(vm, sizeof(AnimationInfoBlockPtr));
        new (pointer)AnimationInfoBlockPtr(ptr);

        sq_settypetag(vm, -1, AnimationInfoTypeTag);
        sq_setreleasehook(vm, -1, AnimationObjectReleaseHook);
    }

    UserDataGetResult AnimationInfoUserData::readBlockPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, AnimationInfoBlockPtr* outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != AnimationInfoTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        AnimationInfoBlockPtr* p = static_cast<AnimationInfoBlockPtr*>(pointer);
        *outObject = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger AnimationInfoUserData::AnimationObjectReleaseHook(SQUserPointer p, SQInteger size){
        AnimationInfoBlockPtr* ptr = static_cast<AnimationInfoBlockPtr*>(p);
        ptr->reset();

        return 0;
    }

}
