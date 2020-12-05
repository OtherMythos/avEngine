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
        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger subscribe(HSQUIRRELVM vm);
        static SQInteger unsubscribe(HSQUIRRELVM vm);
        static SQInteger transmit(HSQUIRRELVM vm);
    };
}
