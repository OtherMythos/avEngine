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
        static SQInteger grabCursor(HSQUIRRELVM vm);
        static SQInteger getRenderTexture(HSQUIRRELVM vm);
    };
}
