#include "WorkerSelfNamespace.h"

#include "../ScriptWorker.h"
#include "../ScriptWorkerVM.h"

namespace AV{

    SQInteger WorkerSelfNamespace::setProgress(HSQUIRRELVM vm){
        ScriptWorker* worker = ScriptWorkerVM::getOwner(vm);
        if(!worker) return 0;

        SQFloat value = 0.0f;
        sq_getfloat(vm, 2, &value);

        if(value < 0.0f) value = 0.0f;
        else if(value > 1.0f) value = 1.0f;

        worker->setProgress(static_cast<float>(value));

        return 0;
    }

    SQInteger WorkerSelfNamespace::isCancelled(HSQUIRRELVM vm){
        ScriptWorker* worker = ScriptWorkerVM::getOwner(vm);
        sq_pushbool(vm, worker ? worker->cancelRequested() : SQFalse);

        return 1;
    }

    /**SQNamespace
    @name _workerSelf
    @desc Available only inside a script worker vm. Lets a worker script report back to the main
    thread while it runs.
    */
    void WorkerSelfNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name setProgress
        @desc Publish how far through this run the worker is. Read on the main thread with the
        worker handle's progress(). Clamped to 0..1.
        @param1:Float:Progress between 0 and 1.
        */
        ScriptUtils::addFunction(vm, setProgress, "setProgress", 2, ".n");

        /**SQFunction
        @name isCancelled
        @desc True once the main thread has destroyed this worker, or the engine has begun
        shutting down. A run which loops for any length of time should check this and return
        early, otherwise it will hold up engine shutdown.
        @returns A boolean.
        */
        ScriptUtils::addFunction(vm, isCancelled, "isCancelled");
    }

    void WorkerSelfNamespace::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_WORKER_IDLE", (SQInteger)ScriptWorker::State::Idle);
        ScriptUtils::declareConstant(vm, "_WORKER_DISPATCHED", (SQInteger)ScriptWorker::State::Dispatched);
        ScriptUtils::declareConstant(vm, "_WORKER_RUNNING", (SQInteger)ScriptWorker::State::Running);
        ScriptUtils::declareConstant(vm, "_WORKER_READY", (SQInteger)ScriptWorker::State::ResultReady);
        ScriptUtils::declareConstant(vm, "_WORKER_FAILED", (SQInteger)ScriptWorker::State::Failed);
        ScriptUtils::declareConstant(vm, "_WORKER_DESTROYED", (SQInteger)ScriptWorker::State::Destroyed);
    }
}
