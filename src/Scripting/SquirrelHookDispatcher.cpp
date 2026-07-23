#if defined(DEBUGGING_TOOLS) || defined(SCRIPT_PROFILER)

#include "SquirrelHookDispatcher.h"

namespace AV{
    static const int NUM_CONSUMERS = static_cast<int>(SquirrelHookDispatcher::Consumer::MAX);

    HSQUIRRELVM SquirrelHookDispatcher::mVm = 0;
    SquirrelHookDispatcher::HookFunction SquirrelHookDispatcher::mConsumers[NUM_CONSUMERS] = { 0 };
    bool SquirrelHookDispatcher::mHookSet = false;

    void SquirrelHookDispatcher::initialise(HSQUIRRELVM vm){
        //Consumers registered against the old vm have no meaning for this one.
        for(int i = 0; i < NUM_CONSUMERS; i++){
            mConsumers[i] = 0;
        }
        mVm = vm;
        //A freshly opened vm has no hook regardless of what the previous one had.
        mHookSet = false;
    }

    void SquirrelHookDispatcher::shutdown(){
        for(int i = 0; i < NUM_CONSUMERS; i++){
            mConsumers[i] = 0;
        }
        if(mVm && mHookSet){
            sq_setnativedebughook(mVm, nullptr);
        }
        mHookSet = false;
        mVm = 0;
    }

    void SquirrelHookDispatcher::setConsumer(Consumer consumer, HookFunction func){
        mConsumers[static_cast<int>(consumer)] = func;

        _updateHook();
    }

    bool SquirrelHookDispatcher::hasConsumer(Consumer consumer){
        return mConsumers[static_cast<int>(consumer)] != 0;
    }

    void SquirrelHookDispatcher::_updateHook(){
        if(!mVm) return;

        bool wanted = false;
        for(int i = 0; i < NUM_CONSUMERS; i++){
            if(mConsumers[i]){
                wanted = true;
                break;
            }
        }

        if(wanted == mHookSet) return;

        sq_setnativedebughook(mVm, wanted ? _dispatch : nullptr);
        mHookSet = wanted;
    }

    void SquirrelHookDispatcher::_dispatch(HSQUIRRELVM vm, SQInteger type, const SQChar* sourceName, SQInteger line, const SQChar* funcName){
        //Squirrel disables the hook for the duration of this call, so a consumer which
        //re-enters the vm (the debugger reads locals, the profiler interns strings)
        //cannot recurse back in here.
        for(int i = 0; i < NUM_CONSUMERS; i++){
            HookFunction func = mConsumers[i];
            if(func){
                (func)(vm, type, sourceName, line, funcName);
            }
        }
    }
}

#endif
