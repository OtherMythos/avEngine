#pragma once

#include <string>
#include <vector>

namespace AV{
    class CallbackScript;

    class ScriptingStateManager{
    public:
        ScriptingStateManager();
        ~ScriptingStateManager();
        void initialise();
        void shutdown();

        void update();

        bool startState(const std::string &stateName, const std::string &scriptPath);
        bool stopState(const std::string &stateName);

    private:
        enum class stateEntryStatus{
            STATE_STARTING,
            STATE_RUNNING,
            STATE_ENDING
        };

        struct stateEntry{
            CallbackScript* s;
            std::string stateName;
            stateEntryStatus stateStatus;
        };


        void _callShutdown(stateEntry& state);
        void _destroyStateEntry(stateEntry& state);

        static const std::string engineStateName;

        std::vector<stateEntry> mStates;
    };
}
