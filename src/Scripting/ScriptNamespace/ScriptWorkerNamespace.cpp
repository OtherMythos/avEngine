#include "ScriptWorkerNamespace.h"

#include "Classes/ScriptWorkerHandleUserData.h"
#include "Scripting/Worker/ScriptWorkerManager.h"
#include "Scripting/Worker/WorkerNamespace/WorkerSelfNamespace.h"

namespace AV{

    ScriptWorkerManager* ScriptWorkerNamespace::mManager = 0;

    SQInteger ScriptWorkerNamespace::createWorker(HSQUIRRELVM vm){
        if(!mManager) return sq_throwerror(vm, "The script worker system is not available.");

        const SQChar* path;
        sq_getstring(vm, 2, &path);

        std::string error;
        const ScriptWorkerId id = mManager->create(path, error);
        if(!id.valid()){
            std::string message("_worker.create: ");
            message += error;
            return sq_throwerror(vm, message.c_str());
        }

        ScriptWorkerHandleUserData::workerIdToUserData(vm, id);

        return 1;
    }

    /**SQNamespace
    @name _worker
    @desc Run squirrel on a worker thread, in a second, deliberately reduced vm.

    A worker is given a script file once and keeps it loaded, so state the script stores survives
    between runs - which is what makes this usable for a generator whose output depends on
    everything it has already produced, not only for pure functions.

    The worker vm has no scene tree, no entity system, no gui and no Ogre. It has the squirrel
    standard libraries, file loading, json, a per worker _random, and _workerSelf. Data goes in as
    a table, data comes back as whatever run() returns.

    Only present when ScriptWorkers is enabled in the setup file, so
    ("_worker" in getroottable()) is the check for whether the feature is available at all.
    */
    void ScriptWorkerNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name create
        @desc Load a script file into a new worker vm. If the file declares a setup() function it
        is called once now, on the main thread. The file must declare a run(input) function, which
        is what each dispatch calls.
        @param1:String:A res path to the worker script.
        @returns A worker handle.
        */
        ScriptUtils::addFunction(vm, createWorker, "create", 2, ".s");
    }

    void ScriptWorkerNamespace::setupConstants(HSQUIRRELVM vm){
        //Same values the worker vm declares, since poll() returns one of them.
        WorkerSelfNamespace::setupConstants(vm);
    }
}
