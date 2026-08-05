#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    /**
    _workerSelf, the worker vm's view of the worker it is running in.

    Everything here is a hook back to the main thread. Deliberately tiny - a worker script's job
    is to compute and return, not to reach into the engine.
    */
    class WorkerSelfNamespace{
    public:
        WorkerSelfNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

        /**
        The _WORKER_* state constants. Declared in the main vm as well, since poll() returns one
        of them, so this is the single definition of the values.
        */
        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger setProgress(HSQUIRRELVM vm);
        static SQInteger isCancelled(HSQUIRRELVM vm);
    };
}
