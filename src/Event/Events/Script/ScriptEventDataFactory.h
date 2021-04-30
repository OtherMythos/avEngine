#pragma once

#include "squirrel.h"
#include "Event/Events/Script/ScriptEventDataFactory.h"

namespace AV{
    class Event;

    /**
    Contains static functions to produce squirrel data for an event.
    */
    class ScriptEventDataFactory{
    public:
        ScriptEventDataFactory() = delete;
        ~ScriptEventDataFactory() = delete;

        /**
        Produce a squirrel object to be pushed depending on an event type.
        Depending on the event the object could be anything from a table to an integer.
        */
        static SQObject produceDataForEvent(HSQUIRRELVM vm, const Event& e);
    };
}
