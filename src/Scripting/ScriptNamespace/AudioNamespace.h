#pragma once

#include "ScriptUtils.h"

namespace AV{
    class AudioNamespace{
    public:
        AudioNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger play(HSQUIRRELVM vm);
        static SQInteger pause(HSQUIRRELVM vm);

        static SQInteger getListenerPosition(HSQUIRRELVM vm);
        static SQInteger setListenerPosition(HSQUIRRELVM vm);
        static SQInteger getListenerVelocity(HSQUIRRELVM vm);
        static SQInteger setListenerVelocity(HSQUIRRELVM vm);

        static SQInteger newAudioSource(HSQUIRRELVM vm);

        static SQInteger getVolume(HSQUIRRELVM vm);
        static SQInteger setVolume(HSQUIRRELVM vm);
    };
}
