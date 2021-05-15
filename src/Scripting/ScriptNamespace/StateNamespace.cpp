#include "StateNamespace.h"

#include "System/Pause/PauseState.h"

namespace AV{

    SQInteger StateNamespace::setPauseState(HSQUIRRELVM vm){
        SQInteger pauseState;
        sq_getinteger(vm, -1, &pauseState);

        PauseMask mask = static_cast<PauseMask>(pauseState);
        PauseState::setMask(mask);

        return 0;
    }

    SQInteger StateNamespace::getPauseState(HSQUIRRELVM vm){
        PauseMask mask = PauseState::getMask();
        sq_pushinteger(vm, static_cast<PauseMask>(mask));

        return 1;
    }

    /**SQNamespace
    @name _state
    @desc Functions relating to the state of the engine.
    */
    void StateNamespace::setupNamespace(HSQUIRRELVM vm){

        /**SQFunction
        @name setPauseState
        @desc Set the pause state of the engine. The user can selectively pause and start components in the engine using flags.
        @param1:flags: Bitmask representing the pieces of engine functionality that should be paused.
        */
        ScriptUtils::addFunction(vm, setPauseState, "setPauseState", 2, ".i");
        /**SQFunction
        @name getPauseState
        @desc Get the current pause state.
        */
        ScriptUtils::addFunction(vm, getPauseState, "getPauseState");
    }

    void StateNamespace::setupConstants(HSQUIRRELVM vm){
        for(size_t i = 0; i < sizeof(PauseTypeStrs) / sizeof(const char*); i++){
            const char* c = PauseTypeStrs[i];
            ScriptUtils::declareConstant(vm, c, 1u << i);
        }
    }
}
