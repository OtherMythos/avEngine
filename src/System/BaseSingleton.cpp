#include "BaseSingleton.h"

namespace AV{
    std::shared_ptr<ScriptingStateManager> BaseSingleton::mScriptedStateManager;

    void BaseSingleton::initialise(std::shared_ptr<ScriptingStateManager> scriptedStateManager){
        mScriptedStateManager = scriptedStateManager;
    }

    std::shared_ptr<ScriptingStateManager> BaseSingleton::getScriptedStateManager(){
        return mScriptedStateManager;
    }
}
