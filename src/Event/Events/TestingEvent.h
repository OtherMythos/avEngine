#pragma once

#include "Event.h"
#include "OgreString.h"

namespace AV{

    #define AV_TEST_EVENT_CATEGORY(c) virtual TestingEventCategory eventCategory() const { return c; };

    enum class TestingEventCategory{
        Null,
        booleanAssertFailed,
        comparisonAssertFailed,
        testEnd,
        scriptFailure,
        timeoutReached
    };

    class TestingEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_TEST_EVENT_CATEGORY(TestingEventCategory::Null)
    };

    class TestingEventBooleanAssertFailed : public TestingEvent{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_TEST_EVENT_CATEGORY(TestingEventCategory::booleanAssertFailed)

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
        AV_TEST_EVENT_CATEGORY(TestingEventCategory::comparisonAssertFailed)

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
        AV_TEST_EVENT_CATEGORY(TestingEventCategory::testEnd)

        bool successfulEnd = true;

        Ogre::String srcFile = "";
        int lineNum = 0;

    };

    class TestingEventScriptFailure : public TestingEvent{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_TEST_EVENT_CATEGORY(TestingEventCategory::scriptFailure)

        Ogre::String srcFile = "";
        Ogre::String failureReason = "";
        Ogre::String functionName = "";
        int lineNum = 0;
    };

    class TestingEventTimeoutReached : public TestingEvent{
    public:
        AV_EVENT_TYPE(EventType::Testing)
        AV_TEST_EVENT_CATEGORY(TestingEventCategory::timeoutReached)

        //Here for easy access, then you don't have to include a header for systemSettings!
        bool meansFailure = false;

        int totalSeconds = 0;
    };
}
