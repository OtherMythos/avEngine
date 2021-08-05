#pragma once

#include <string>
#include <vector>
#include <memory>

namespace AV{
    class CallbackScript;

    /**
    Manages a script as part of a state.
    States can be started and stopped whenever necessary, and as their lifetime progresses their scripts are called with the appropriate function.
    For instance, the startup script is considered a state managed by the engine.
    */
    class ScriptingStateManager{
    public:
        ScriptingStateManager();
        ~ScriptingStateManager();
        void initialise();
        void shutdown();

        void update();

        /**
        Update the base state.
        Call this before any other functions of the main update loop.
        */
        void updateBaseState();

        /**
        Create and start a state by name.
        @returns True or false depending on whether the state was started correctly.
        */
        bool startState(const std::string &stateName, const std::string &scriptPath);
        /**
        Stop a running state
        @returns True or false depending on whether the state was stopped correctly.
        */
        bool stopState(const std::string &stateName);

    private:
        enum class stateEntryStatus{
            STATE_STARTING,
            STATE_RUNNING,
            STATE_ENDING
        };

        struct StateEntry{
            std::shared_ptr<CallbackScript> s;
            std::string stateName;
            stateEntryStatus stateStatus;
            int startId, updateId, endId;
        };


        void _callShutdown(StateEntry& state);

        static const std::string engineStateName;

        struct{
            StateEntry e;
            int safeSceneUpdate;
        }mBaseStateEntry;

        bool _startBaseState();

        bool _updateStateEntry(StateEntry& state);

        std::vector<StateEntry> mStates;
    };
}
