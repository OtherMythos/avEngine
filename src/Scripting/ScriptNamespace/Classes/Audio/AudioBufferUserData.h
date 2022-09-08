#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Audio/AudioTypes.h"

namespace AV{
    class AudioBufferUserData{
    public:
        AudioBufferUserData() = delete;
        ~AudioBufferUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void audioBufferToUserData(HSQUIRRELVM vm, AudioBufferPtr source);
        static UserDataGetResult readAudioBufferFromUserData(HSQUIRRELVM vm, SQInteger stackInx, AudioBufferPtr* out);

        static SQInteger audioBufferReleaseHook(SQUserPointer p, SQInteger size);

    private:
        static SQObject audioBufferDelegateTableObject;

        static SQInteger audioSourceReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger load(HSQUIRRELVM vm);
    };
}
