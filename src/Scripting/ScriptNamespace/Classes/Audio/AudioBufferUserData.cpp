#include "AudioBufferUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "System/Util/PathUtils.h"

#include "Audio/AudioBuffer.h"

#include <sstream>

namespace AV{

    SQObject AudioBufferUserData::audioBufferDelegateTableObject;

    void AudioBufferUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 11);

        ScriptUtils::addFunction(vm, load, "load", 2, ".s");

        sq_resetobject(&audioBufferDelegateTableObject);
        sq_getstackobj(vm, -1, &audioBufferDelegateTableObject);
        sq_addref(vm, &audioBufferDelegateTableObject);
        sq_pop(vm, 1);
    }

    void AudioBufferUserData::audioBufferToUserData(HSQUIRRELVM vm, AudioBufferPtr buffer){
        AudioBufferPtr* pointer = (AudioBufferPtr*)sq_newuserdata(vm, sizeof(AudioBufferPtr));
        memset(pointer, 0, sizeof(AudioBufferPtr));
        *pointer = buffer;

        sq_pushobject(vm, audioBufferDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, AudioBufferTypeTag);
        sq_setreleasehook(vm, -1, audioBufferReleaseHook);
    }

    SQInteger AudioBufferUserData::load(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, 2, &filePath);
        std::string outString;
        formatResToPath(filePath, outString);

        AudioBufferPtr bufPtr;
        SCRIPT_ASSERT_RESULT(readAudioBufferFromUserData(vm, 1, &bufPtr));

        bufPtr->load(outString);

        return 0;
    }

    SQInteger AudioBufferUserData::audioBufferReleaseHook(SQUserPointer p, SQInteger size){
        AudioBufferPtr* ptr = static_cast<AudioBufferPtr*>(p);
        ptr->reset();

        return 0;
    }

    UserDataGetResult AudioBufferUserData::readAudioBufferFromUserData(HSQUIRRELVM vm, SQInteger stackInx, AudioBufferPtr* out){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != AudioBufferTypeTag){
            *out = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *out = *((AudioBufferPtr*)pointer);

        return USER_DATA_GET_SUCCESS;
    }
}
