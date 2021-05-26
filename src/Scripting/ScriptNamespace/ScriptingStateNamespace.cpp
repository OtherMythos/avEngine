#include "ScriptingStateNamespace.h"

#include "Scripting/ScriptingStateManager.h"
#include "System/Util/PathUtils.h"

namespace AV{
    ScriptingStateManager* ScriptingStateNamespace::stateManager = 0;

    SQInteger ScriptingStateNamespace::startState(HSQUIRRELVM vm){
        const SQChar *stateName;
        sq_getstring(vm, -2, &stateName);

        const SQChar *scriptPath;
        sq_getstring(vm, -1, &scriptPath);
        std::string path;
        formatResToPath(scriptPath, path);

        std::string state(stateName);
        bool result = stateManager->startState(state, path);
        if(!result) return sq_throwerror(vm, "Unable to start scripted state.");

        return 0;
    }

    SQInteger ScriptingStateNamespace::endState(HSQUIRRELVM vm){
        const SQChar *stateName;
        sq_getstring(vm, -1, &stateName);

        std::string state(stateName);
        SQBool result = stateManager->stopState(state);

        sq_pushbool(vm, result);
        return 1;
    }

    void ScriptingStateNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, startState, "startState", 3, ".ss");
        ScriptUtils::addFunction(vm, endState, "endState", 2, ".s");
    }
}
