#include "StateNamespace.h"

#include "System/Pause/PauseState.h"

#include "System/Util/Scene/ParticleSystemTimeHelper.h"

namespace AV{

    SQInteger StateNamespace::setPauseState(HSQUIRRELVM vm){
        SQInteger pauseState;
        sq_getinteger(vm, -1, &pauseState);

        PauseMask mask = static_cast<PauseMask>(pauseState);
        PauseState::setMask(mask);

        bool updateParticles = !(mask & PAUSE_TYPE_PARTICLES);
        ParticleSystemTimeHelper::setUpdateParticles(updateParticles);

        return 0;
    }

    SQInteger StateNamespace::getPauseState(HSQUIRRELVM vm){
        PauseMask mask = PauseState::getMask();
        sq_pushinteger(vm, static_cast<PauseMask>(mask));

        return 1;
    }

    SQInteger StateNamespace::setDefaultAnimationPauseMask(HSQUIRRELVM vm){
        SQInteger maskValue;
        sq_getinteger(vm, -1, &maskValue);

        uint32 mask = static_cast<uint32>(maskValue);
        PauseState::setDefaultAnimationPauseMask(mask);

        return 0;
    }

    SQInteger StateNamespace::getDefaultAnimationPauseMask(HSQUIRRELVM vm){
        uint32 mask = PauseState::getDefaultAnimationPauseMask();
        sq_pushinteger(vm, static_cast<SQInteger>(mask));

        return 1;
    }

    SQInteger StateNamespace::setAnimationPauseMask(HSQUIRRELVM vm){
        SQInteger maskValue;
        sq_getinteger(vm, -1, &maskValue);

        uint32 mask = static_cast<uint32>(maskValue);
        PauseState::setPauseAnimationMask(mask);

        return 0;
    }

    SQInteger StateNamespace::getAnimationPauseMask(HSQUIRRELVM vm){
        uint32 mask = PauseState::getPauseAnimationMask();
        sq_pushinteger(vm, static_cast<SQInteger>(mask));

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
        @returns A bitmask representing the current pause state.
        */
        ScriptUtils::addFunction(vm, getPauseState, "getPauseState");

        /**SQFunction
        @name setDefaultAnimationPauseMask
        @desc Set the default pause mask for new animations.
        @param1:mask: Bitmask representing the default pause mask.
        */
        ScriptUtils::addFunction(vm, setDefaultAnimationPauseMask, "setDefaultAnimationPauseMask", 2, ".i");
        /**SQFunction
        @name getDefaultAnimationPauseMask
        @desc Get the default animation pause mask.
        @returns A bitmask representing the default animation pause mask.
        */
        ScriptUtils::addFunction(vm, getDefaultAnimationPauseMask, "getDefaultAnimationPauseMask");

        /**SQFunction
        @name setAnimationPauseMask
        @desc Set the global animation pause mask for checking against individual animation masks.
        @param1:mask: Bitmask representing which animations should be paused.
        */
        ScriptUtils::addFunction(vm, setAnimationPauseMask, "setAnimationPauseMask", 2, ".i");
        /**SQFunction
        @name getAnimationPauseMask
        @desc Get the global animation pause mask.
        @returns A bitmask representing the current animation pause mask.
        */
        ScriptUtils::addFunction(vm, getAnimationPauseMask, "getAnimationPauseMask");
    }

    void StateNamespace::setupConstants(HSQUIRRELVM vm){


        for(size_t i = 0; i < sizeof(PauseTypeStrs) / sizeof(const char*); i++){
            const char* c = PauseTypeStrs[i];
            ScriptUtils::declareConstant(vm, c, 1u << i);
        }
    }
}
