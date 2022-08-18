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

        static SQInteger newAudioSource(HSQUIRRELVM vm);
    };
}
