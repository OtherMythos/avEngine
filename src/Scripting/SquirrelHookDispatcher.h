#if defined(DEBUGGING_TOOLS) || defined(SCRIPT_PROFILER)

#pragma once

#include <squirrel.h>

namespace AV{

    /**
    Owns the single native debug hook a Squirrel vm provides, and fans its events out to
    each interested consumer.

    Squirrel allows exactly one hook per vm (sq_setnativedebughook), so the script
    debugger and the script profiler cannot each install their own; whichever set it last
    would silently disable the other. Both register here instead.

    The native hook is only installed on the vm while at least one consumer is registered,
    so a vm with neither pays nothing.
    */
    class SquirrelHookDispatcher{
    public:
        SquirrelHookDispatcher() = delete;
        ~SquirrelHookDispatcher() = delete;

        enum class Consumer{
            DEBUGGER,
            PROFILER,

            MAX
        };

        typedef void(*HookFunction)(HSQUIRRELVM vm, SQInteger type, const SQChar* sourceName, SQInteger line, const SQChar* funcName);

        /**
        Point the dispatcher at a vm.

        Any consumers registered against a previous vm are dropped, as their state
        belonged to that vm. ScriptVM calls this each time it opens a vm.
        */
        static void initialise(HSQUIRRELVM vm);

        /**
        Detach from the current vm and drop all consumers.
        Must be called before the vm is closed.
        */
        static void shutdown();

        /**
        Register a consumer to receive hook events, or pass 0 to unregister it.
        */
        static void setConsumer(Consumer consumer, HookFunction func);

        static bool hasConsumer(Consumer consumer);

    private:
        static HSQUIRRELVM mVm;
        static HookFunction mConsumers[static_cast<int>(Consumer::MAX)];
        static bool mHookSet;

        static void _dispatch(HSQUIRRELVM vm, SQInteger type, const SQChar* sourceName, SQInteger line, const SQChar* funcName);

        /**
        Install or remove the native hook depending on whether any consumer wants events.
        */
        static void _updateHook();
    };
}

#endif
