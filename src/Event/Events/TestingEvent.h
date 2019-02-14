#pragma once

#include "Event.h"
#include "OgreString.h"

namespace AV{

    #define AV_EVENT_CATEGORY(c) virtual TestingEventCategory eventCategory() const { return c; };

    enum class TestingEventCategory{
        Null,
        booleanAssertFailed
    };

    class TestingEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_EVENT_CATEGORY(TestingEventCategory::Null)
    };

    class TestingEventBooleanAssertFailed : public TestingEvent{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_EVENT_CATEGORY(TestingEventCategory::booleanAssertFailed)

        bool expected = false;
        //No need for returned if we have the expected.
        //If the expected if false the test should always have returned true and visa versa.
    };
}
