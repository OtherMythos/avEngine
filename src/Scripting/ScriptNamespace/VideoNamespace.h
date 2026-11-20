#pragma once

#include <squirrel.h>

namespace AV{
    class VideoNamespace{
    public:
        VideoNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger openVideo(HSQUIRRELVM vm);
    };
}
