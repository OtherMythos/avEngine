#include "LottieAnimationUserData.h"

#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptObjectTypeTags.h"

#include "LottieSurfaceUserData.h"

#include "rlottie.h"

namespace AV{
    SQObject LottieAnimationUserData::lottieAnimationDelegateTableObject;

    void LottieAnimationUserData::lottieAnimationToUserData(HSQUIRRELVM vm, LottieAnimationPtr animation){
        LottieAnimationPtr* pointer = (LottieAnimationPtr*)sq_newuserdata(vm, sizeof(LottieAnimationPtr));
        memset(pointer, 0, sizeof(LottieAnimationPtr));
        *pointer = animation;

        sq_settypetag(vm, -1, LottieAnimationTypeTag);
        sq_setreleasehook(vm, -1, sqReleaseHook);
        sq_pushobject(vm, lottieAnimationDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
    }

    UserDataGetResult LottieAnimationUserData::readLottieAnimationFromUserData(HSQUIRRELVM vm, SQInteger stackInx, LottieAnimationPtr* outAnim){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != LottieAnimationTypeTag){
            outAnim->reset();
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outAnim = *((LottieAnimationPtr*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger LottieAnimationUserData::totalFrame(HSQUIRRELVM vm){
        LottieAnimationPtr ptr;
        SCRIPT_ASSERT_RESULT(readLottieAnimationFromUserData(vm, 1, &ptr));

        sq_pushinteger(vm, ptr->totalFrame());

        return 1;
    }

    SQInteger LottieAnimationUserData::renderSync(HSQUIRRELVM vm){
        LottieAnimationPtr ptr;
        SCRIPT_ASSERT_RESULT(readLottieAnimationFromUserData(vm, 1, &ptr));

        LottieSurfacePtr surfacePtr;
        SCRIPT_CHECK_RESULT(LottieSurfaceUserData::readLottieSurfaceFromUserData(vm, 2, &surfacePtr));

        SQInteger frameId;
        sq_getinteger(vm, 3, &frameId);

        rlottie::Surface* surface = surfacePtr->surface.get();
        ptr->renderSync(frameId, *surface);

        return 0;
    }

    SQInteger LottieAnimationUserData::sqReleaseHook(SQUserPointer p, SQInteger size){
        LottieAnimationPtr* ptr = static_cast<LottieAnimationPtr*>(p);
        ptr->reset();

        return 0;
    }

    SQInteger LottieAnimationUserData::lottieAnimationToString(HSQUIRRELVM vm){
        LottieAnimationPtr ptr;
        SCRIPT_ASSERT_RESULT(readLottieAnimationFromUserData(vm, 1, &ptr));

        std::ostringstream stream;
        stream << "LottieAnimation (\"" << ptr.get() << "\")";
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    void LottieAnimationUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, totalFrame, "totalFrame");
        ScriptUtils::addFunction(vm, renderSync, "renderSync", 3, ".ui");
        ScriptUtils::addFunction(vm, lottieAnimationToString, "_tostring");

        sq_resetobject(&lottieAnimationDelegateTableObject);
        sq_getstackobj(vm, -1, &lottieAnimationDelegateTableObject);
        sq_addref(vm, &lottieAnimationDelegateTableObject);
        sq_pop(vm, 1);
    }
}
