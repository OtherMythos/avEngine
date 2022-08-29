#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Audio/AudioTypes.h"

namespace AV{
    class AudioSourceUserData{
    public:
        AudioSourceUserData() = delete;
        ~AudioSourceUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void audioSourceToUserData(HSQUIRRELVM vm, AudioSourcePtr source);
        static UserDataGetResult readAudioSourceFromUserData(HSQUIRRELVM vm, SQInteger stackInx, AudioSourcePtr* out);

    private:
        static SQObject audioSourceDelegateTableObject;

        static SQInteger play(HSQUIRRELVM vm);
        static SQInteger pause(HSQUIRRELVM vm);
        static SQInteger getAudioBuffer(HSQUIRRELVM vm);
        static SQInteger setPosition(HSQUIRRELVM vm);

        static SQInteger audioSourceReleaseHook(SQUserPointer p, SQInteger size);
    };
}
