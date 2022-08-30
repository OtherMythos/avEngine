#include "AudioSourceUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"

#include "Audio/AudioSource.h"

#include "AudioBufferUserData.h"

#include <sstream>

namespace AV{

    SQObject AudioSourceUserData::audioSourceDelegateTableObject;

    void AudioSourceUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 11);

        ScriptUtils::addFunction(vm, play, "play");
        ScriptUtils::addFunction(vm, pause, "pause");
        ScriptUtils::addFunction(vm, stop, "stop");
        ScriptUtils::addFunction(vm, getAudioBuffer, "getAudioBuffer");
        ScriptUtils::addFunction(vm, setPosition, "setPosition", -2, ".n|unn");
        ScriptUtils::addFunction(vm, setLooping, "setLooping", 2, ".b");
        ScriptUtils::addFunction(vm, setVolume, "setVolume", 2, ".n");

        sq_resetobject(&audioSourceDelegateTableObject);
        sq_getstackobj(vm, -1, &audioSourceDelegateTableObject);
        sq_addref(vm, &audioSourceDelegateTableObject);
        sq_pop(vm, 1);
    }

    void AudioSourceUserData::audioSourceToUserData(HSQUIRRELVM vm, AudioSourcePtr source){
        AudioSourcePtr* pointer = (AudioSourcePtr*)sq_newuserdata(vm, sizeof(AudioSourcePtr));
        memset(pointer, 0, sizeof(AudioSourcePtr));
        *pointer = source;

        sq_pushobject(vm, audioSourceDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, AudioSourceTypeTag);
        sq_setreleasehook(vm, -1, audioSourceReleaseHook);
    }

    SQInteger AudioSourceUserData::setLooping(HSQUIRRELVM vm){
        AudioSourcePtr outPtr;
        SCRIPT_ASSERT_RESULT(readAudioSourceFromUserData(vm, 1, &outPtr));

        SQBool looping;
        sq_getbool(vm, 2, &looping);

        outPtr->setLooping(looping);

        return 0;
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

    SQInteger AudioSourceUserData::stop(HSQUIRRELVM vm){
        AudioSourcePtr outPtr;
        SCRIPT_ASSERT_RESULT(readAudioSourceFromUserData(vm, 1, &outPtr));

        outPtr->stop();

        return 0;
    }

    SQInteger AudioSourceUserData::setVolume(HSQUIRRELVM vm){
        AudioSourcePtr outPtr;
        SCRIPT_ASSERT_RESULT(readAudioSourceFromUserData(vm, 1, &outPtr));

        SQFloat volume;
        sq_getfloat(vm, 2, &volume);

        outPtr->setVolume(volume);

        return 0;
    }

    SQInteger AudioSourceUserData::setPosition(HSQUIRRELVM vm){
        AudioSourcePtr outPtr;
        SCRIPT_ASSERT_RESULT(readAudioSourceFromUserData(vm, 1, &outPtr));

        Ogre::Vector3 outVec;
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &outVec);
        if(result != 0) return result;

        outPtr->setPosition(outVec);

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
