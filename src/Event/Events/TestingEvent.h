#ifdef TEST_MODE

#pragma once

#include "Event.h"
#include "OgreString.h"

namespace AV{
    class TestingEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_EVENT_ID(EventId::Null)
    };

    class TestingEventBooleanAssertFailed : public TestingEvent{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_EVENT_ID(EventId::TestingBooleanAssertFailed)

        bool expected = false;
        //No need for returned if we have the expected.
        //If the expected if false the test should always have returned true and visa versa.

        int lineNum = 0;
        Ogre::String srcFile = "";
        Ogre::String functionName = "";
        Ogre::String codeLine = "";
    };

    class TestingEventComparisonAssertFailed : public TestingEvent{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_EVENT_ID(EventId::TestingComparisonAssertFailed)

        //Whether this was an equals comparison or a not equals comparison.
        bool equals = true;

        Ogre::String firstType = "";
        Ogre::String firstValue = "";
        Ogre::String secondType = "";
        Ogre::String secondValue = "";

        int lineNum = 0;
        Ogre::String srcFile = "";
        Ogre::String functionName = "";
        Ogre::String codeLine = "";
    };

    class TestingEventTestEnd : public TestingEvent{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_EVENT_ID(EventId::TestingTestEnd)

        bool successfulEnd = true;

        Ogre::String srcFile = "";
        int lineNum = 0;

    };

    class TestingEventScriptFailure : public TestingEvent{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_EVENT_ID(EventId::TestingScriptFailure)

        Ogre::String srcFile = "";
        Ogre::String failureReason = "";
        Ogre::String functionName = "";
        int lineNum = 0;
    };

    class TestingEventTimeoutReached : public TestingEvent{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_EVENT_ID(EventId::TestingTimeoutReached)

        //Here for easy access, then you don't have to include a header for systemSettings!
        bool meansFailure = false;

        int totalSeconds = 0;
    };
}

#endif
