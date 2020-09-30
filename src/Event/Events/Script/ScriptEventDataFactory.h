#pragma once

#include "squirrel.h"
#include "Event/Events/Script/ScriptEventDataFactory.h"

namespace AV{
    class Event;

    class ScriptEventDataFactory{
    public:
        ScriptEventDataFactory() = delete;
        ~ScriptEventDataFactory() = delete;

        static SQObject produceDataForEvent(HSQUIRRELVM vm, const Event& e);
    };
}
