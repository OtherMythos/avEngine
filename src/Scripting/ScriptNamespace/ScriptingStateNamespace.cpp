#include "ScriptingStateNamespace.h"

#include "Scripting/ScriptingStateManager.h"

namespace AV{
    ScriptingStateManager* ScriptingStateNamespace::stateManager = 0;

    SQInteger ScriptingStateNamespace::startState(HSQUIRRELVM vm){
        const SQChar *stateName;
        sq_getstring(vm, -2, &stateName);

        const SQChar *scriptPath;
        sq_getstring(vm, -1, &scriptPath);

        std::string state(stateName);
        std::string path(scriptPath);
        SQBool result = stateManager->startState(state, path);

        sq_pushbool(vm, result);
        return 1;
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
        _addFunction(vm, startState, "startState", 3, ".ss");
        _addFunction(vm, endState, "endState", 2, ".s");
    }
}
