#pragma once

#include <vector>
#include <string>
#include <chrono>

namespace AV{
    class Event;
    class TestingEvent;

    class TestModeManager{
    public:
        TestModeManager();
        virtual ~TestModeManager();

        void initialise();

        bool systemEventReceiver(const Event &e);
        bool testEventReceiver(const Event &e);
        
        void updateTimeout();

    private:
        void _createTestFile(const std::string &path);
        void _failTest(const std::vector<std::string>& failMessage);
        /**
        End the test, writing the finished status of the test to the file.
        */
        void _endTest();
        /**
        Print the failure message to the console.

        @param message
        A list of messages to be printed.
        */
        void _printTestFailureMessage(const std::vector<std::string>& message);
        /**
        Print a test success message.
        */
        void _printTestSuccessMessage();
        /**
        Create and return a vector containing a failure message based on a testing event.
        These strings can be printed or dumpted to a file.

        @param e
        The testing event to determine the error message from.

        @return
        A vector containing strings as lines for the failure message.
        */
        std::vector<std::string> _getFailureMessage(const TestingEvent &e);

        void _eventFailTest(const TestingEvent& testEvent);
        
        void _processTimeout(bool meansFail, int time);

        //Whether or not the test failed.
        bool testFailed = false;
        //Whether the test finished, ambiguous of whether it was a pass or fail.
        bool testFinished = false;

        std::string testFilePath;
        
        std::chrono::time_point<std::chrono::system_clock> startTime;
    };
}
