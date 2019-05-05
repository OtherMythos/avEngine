#pragma once

#include <memory>

namespace AV{
    class Base;
    class ScriptingStateManager;

    class BaseSingleton{
        friend class Base;
    public:
        std::shared_ptr<ScriptingStateManager> getScriptedStateManager();
    private:
        static void initialise(std::shared_ptr<ScriptingStateManager> scriptedStateManager);

        static std::shared_ptr<ScriptingStateManager> mScriptedStateManager;
    };
}
