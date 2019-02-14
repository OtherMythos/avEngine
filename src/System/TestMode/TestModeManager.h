#pragma once

#include <string>

namespace AV{
    class Event;

    class TestModeManager{
    public:
        TestModeManager();
        virtual ~TestModeManager();

        void initialise();

        bool systemEventReceiver(const Event &e);
        bool testEventReceiver(const Event &e);

    private:
        void _createTestFile(const std::string &path);
        void _failTest();
        void _endTest();

        bool testFailed = false;

        std::string testFilePath;
    };
}
