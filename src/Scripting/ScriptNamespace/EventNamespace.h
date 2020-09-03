#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ScriptVM;
    class ScriptEventManager;

    class EventNamespace{
        friend ScriptVM;
    public:
        EventNamespace() = delete;

        static ScriptEventManager* _scriptEventManager;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger subscribe(HSQUIRRELVM vm);
    };
}
