#include "AudioSourceUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "Audio/AudioSource.h"

#include "AudioBufferUserData.h"

#include <sstream>

namespace AV{

    SQObject AudioSourceUserData::audioSourceDelegateTableObject;

    void AudioSourceUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 11);

        ScriptUtils::addFunction(vm, play, "play");
        ScriptUtils::addFunction(vm, pause, "pause");
        ScriptUtils::addFunction(vm, getAudioBuffer, "getAudioBuffer");

        sq_resetobject(&audioSourceDelegateTableObject);
        sq_getstackobj(vm, -1, &audioSourceDelegateTableObject);
        sq_addref(vm, &audioSourceDelegateTableObject);
        sq_pop(vm, 1);
    }

    void AudioSourceUserData::audioSourceToUserData(HSQUIRRELVM vm, AudioSourcePtr source){
        AudioSourcePtr* pointer = (AudioSourcePtr*)sq_newuserdata(vm, sizeof(AudioSourcePtr));
        *pointer = source;

        sq_pushobject(vm, audioSourceDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, AudioSourceTypeTag);
        sq_setreleasehook(vm, -1, audioSourceReleaseHook);
    }

    SQInteger AudioSourceUserData::play(HSQUIRRELVM vm){
        AudioSourcePtr outPtr;
        SCRIPT_ASSERT_RESULT(readAudioSourceFromUserData(vm, 1, &outPtr));

        outPtr->play();

        return 0;
    }

    SQInteger AudioSourceUserData::pause(HSQUIRRELVM vm){
        AudioSourcePtr outPtr;
        SCRIPT_ASSERT_RESULT(readAudioSourceFromUserData(vm, 1, &outPtr));

        outPtr->pause();

        return 0;
    }

    SQInteger AudioSourceUserData::getAudioBuffer(HSQUIRRELVM vm){
        AudioSourcePtr outPtr;
        SCRIPT_ASSERT_RESULT(readAudioSourceFromUserData(vm, 1, &outPtr));

        AudioBufferPtr bufPtr = outPtr->getAudioBuffer();
        AudioBufferUserData::audioBufferToUserData(vm, bufPtr);

        return 1;
    }

    SQInteger AudioSourceUserData::audioSourceReleaseHook(SQUserPointer p, SQInteger size){
        AudioSourcePtr* ptr = static_cast<AudioSourcePtr*>(p);
        ptr->reset();

        return 0;
    }

    UserDataGetResult AudioSourceUserData::readAudioSourceFromUserData(HSQUIRRELVM vm, SQInteger stackInx, AudioSourcePtr* out){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != AudioSourceTypeTag){
            *out = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *out = *((AudioSourcePtr*)pointer);

        return USER_DATA_GET_SUCCESS;
    }
}
