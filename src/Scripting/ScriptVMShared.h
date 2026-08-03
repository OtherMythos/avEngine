#pragma once

#include <squirrel.h>
#include <string>

namespace AV{

    /**
    Pieces of virtual machine setup which are not specific to the main vm.

    The engine runs more than one squirrel vm - the main one, the throwaway vm used for the setup
    function, and a vm per script worker thread. All of them want the same print callback and the
    same error reporting, but only the main vm should attach the debugger or fail a test run. That
    policy stays in ScriptVM; the reporting lives here so a second vm does not have to duplicate it.

    Everything in this file is safe to call from any thread. The logger sinks are thread safe and
    none of these functions touch engine state.
    */

    /**
    Squirrel's print callback. Routes script print() output to the squirrel logger.
    */
    void sqPrintFunc(HSQUIRRELVM v, const SQChar* s, ...);

    /**
    Log the vm's current error along with the frame it happened in.

    @param vm The vm whose error handler is running.
    @param vmTag Identifies which vm this is in the log, i.e. "main" or "worker", so failures from
                 a worker thread can be told apart from the main thread's.
    @param outMessage If provided, receives the error text on its own, for a caller which wants to
                      surface it somewhere other than the log.
    @param outStackInfo If provided, receives the frame the error occurred in, saving the caller a
                        second sq_stackinfos call.
    */
    void sqLogRuntimeError(HSQUIRRELVM vm, const char* vmTag, std::string* outMessage = 0, SQStackInfos* outStackInfo = 0);

    /**
    Log a compilation failure. Parameters mirror SQCOMPILERERROR.
    */
    void sqLogCompilerError(const char* vmTag, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column);
}
