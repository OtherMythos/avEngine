#pragma once

#include <string>

namespace AV{
    class Event;

    class TestModeManager{
    public:
        TestModeManager();
        virtual ~TestModeManager();

        void initialise();

        bool worldEventReceiver(const Event &e);

    private:
        void _createTestFile(const std::string &path);

        std::string testFilePath;
    };
}
