#pragma once

#include <vector>
#include <string>

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

    private:
        void _createTestFile(const std::string &path);
        void _failTest(const std::vector<std::string>& failMessage);
        void _endTest();
        void _printTestFailureMessage(const std::vector<std::string>& message);
        void _printTestSuccessMessage();
        std::vector<std::string> _getFailureMessage(const TestingEvent &e);

        bool testFailed = false;

        std::string testFilePath;
    };
}
