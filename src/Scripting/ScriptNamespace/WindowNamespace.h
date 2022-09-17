#pragma once

#include "ScriptUtils.h"

namespace AV{
    class WindowNamespace{
    public:
        WindowNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger getWidth(HSQUIRRELVM vm);
        static SQInteger getHeight(HSQUIRRELVM vm);
        static SQInteger getActualWidth(HSQUIRRELVM vm);
        static SQInteger getActualHeight(HSQUIRRELVM vm);
        static SQInteger grabCursor(HSQUIRRELVM vm);
        static SQInteger showCursor(HSQUIRRELVM vm);
        static SQInteger getRenderTexture(HSQUIRRELVM vm);
        static SQInteger warpMouseInWindow(HSQUIRRELVM vm);
        static SQInteger setFullscreen(HSQUIRRELVM vm);
        static SQInteger getFullscreen(HSQUIRRELVM vm);
    };
}
