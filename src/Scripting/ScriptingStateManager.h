#pragma once

#include <string>
#include <vector>

namespace AV{
    class ScriptingStateManager{
    public:
        ScriptingStateManager();
        ~ScriptingStateManager();
        void initialise();
        void shutdown();

        void update();

        bool startState(const std::string stateName, const std::string scriptPath);
        bool stopState(const std::string stateName);

    private:
        enum class stateEntryStatus{
            STATE_STARTING,
            STATE_RUNNING,
            STATE_ENDING
        };

        struct stateEntry{
            std::string scriptFile;
            std::string stateName;
            stateEntryStatus stateStatus;
        };


        void _callShutdown(const stateEntry& state);

        static const std::string engineStateName;

        std::vector<stateEntry> mStates;
    };
}
