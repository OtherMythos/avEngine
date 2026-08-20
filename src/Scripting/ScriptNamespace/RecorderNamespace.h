#ifdef FLIGHT_RECORDER

#pragma once

#include "ScriptUtils.h"

namespace AV{
    class RecorderNamespace{
    public:
        RecorderNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger capture(HSQUIRRELVM vm);
        static SQInteger mark(HSQUIRRELVM vm);
        static SQInteger watch(HSQUIRRELVM vm);
        static SQInteger unwatch(HSQUIRRELVM vm);
        static SQInteger getWatches(HSQUIRRELVM vm);
        static SQInteger isEnabled(HSQUIRRELVM vm);
        static SQInteger start(HSQUIRRELVM vm);
        static SQInteger stop(HSQUIRRELVM vm);
        static SQInteger getFramesBuffered(HSQUIRRELVM vm);
        static SQInteger getLastCapturePath(HSQUIRRELVM vm);
    };
}

#endif
