#include "AnimationInstanceUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    void AnimationInstanceUserData::animationPtrToUserData(HSQUIRRELVM vm, SequenceAnimationPtr ptr){
        SequenceAnimationPtr* pointer = (SequenceAnimationPtr*)sq_newuserdata(vm, sizeof(SequenceAnimationPtr));
        new (pointer)SequenceAnimationPtr(ptr);

        sq_settypetag(vm, -1, AnimationInstanceTypeTag);
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

}
