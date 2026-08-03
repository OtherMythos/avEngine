#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ScriptWorkerManager;

    /**
    _worker, the main vm's entry point into the script worker thread system.

    Only registered when ScriptWorkers is enabled in the setup file, so
    ("_worker" in getroottable()) is how a script checks whether the feature is available. That
    follows what _test, _developer and _monetisation already do, and means a project which does
    not use workers carries none of this.
    */
    class ScriptWorkerNamespace{
    public:
        ScriptWorkerNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        //Set by ScriptWorkerManager's constructor and cleared by its shutdown, so a call arriving
        //after teardown throws rather than reaching a destroyed manager.
        static ScriptWorkerManager* mManager;

    private:
        static SQInteger createWorker(HSQUIRRELVM vm);
    };
}
