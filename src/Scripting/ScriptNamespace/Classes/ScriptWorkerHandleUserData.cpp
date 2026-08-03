#include "ScriptWorkerHandleUserData.h"

#include "Scripting/ScriptNamespace/ScriptWorkerNamespace.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/Worker/ScriptWorkerManager.h"

namespace AV{

    SQObject ScriptWorkerHandleUserData::workerDelegateTableObject;

    void ScriptWorkerHandleUserData::workerIdToUserData(HSQUIRRELVM vm, ScriptWorkerId id){
        ScriptWorkerId* pointer = (ScriptWorkerId*)sq_newuserdata(vm, sizeof(ScriptWorkerId));
        *pointer = id;

        //No release hook. See the class comment.
        sq_pushobject(vm, workerDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, ScriptWorkerHandleTypeTag);
    }

    UserDataGetResult ScriptWorkerHandleUserData::readWorkerIdFromUserData(HSQUIRRELVM vm, SQInteger stackInx, ScriptWorkerId* outId){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != ScriptWorkerHandleTypeTag){
            *outId = ScriptWorkerId();
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outId = *((ScriptWorkerId*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    ScriptWorker* ScriptWorkerHandleUserData::_resolve(HSQUIRRELVM vm, ScriptWorkerManager** outManager){
        if(outManager) *outManager = ScriptWorkerNamespace::mManager;
        if(!ScriptWorkerNamespace::mManager) return 0;

        ScriptWorkerId id;
        if(readWorkerIdFromUserData(vm, 1, &id) != USER_DATA_GET_SUCCESS) return 0;

        return ScriptWorkerNamespace::mManager->get(id);
    }

    SQInteger ScriptWorkerHandleUserData::dispatch(HSQUIRRELVM vm){
        ScriptWorker* worker = _resolve(vm, 0);
        if(!worker) return sq_throwerror(vm, "This worker handle is no longer valid.");

        if(worker->state() != ScriptWorker::State::Idle){
            sq_pushbool(vm, SQFalse);
            return 1;
        }

        std::string error;
        //The input is at index 2. dispatch copies it into the worker vm before queueing anything,
        //so a rejected input leaves the worker exactly as it was.
        if(!worker->dispatch(vm, 2, error)){
            std::string message("_worker: ");
            message += error;
            return sq_throwerror(vm, message.c_str());
        }

        sq_pushbool(vm, SQTrue);
        return 1;
    }

    SQInteger ScriptWorkerHandleUserData::poll(HSQUIRRELVM vm){
        ScriptWorker* worker = _resolve(vm, 0);
        //A destroyed worker is a legitimate state to observe rather than an error, so that a
        //polling loop does not have to be wrapped in a try.
        sq_pushinteger(vm, worker ? (SQInteger)worker->state() : (SQInteger)ScriptWorker::State::Destroyed);

        return 1;
    }

    SQInteger ScriptWorkerHandleUserData::claim(HSQUIRRELVM vm){
        ScriptWorker* worker = _resolve(vm, 0);
        if(!worker) return sq_throwerror(vm, "This worker handle is no longer valid.");

        const ScriptWorker::State state = worker->state();
        if(state != ScriptWorker::State::ResultReady && state != ScriptWorker::State::Failed){
            //Nothing to take yet. The common case in a per frame poll, so not an error.
            sq_pushnull(vm);
            return 1;
        }

        std::string error;
        if(!worker->claim(vm, error)){
            std::string message("_worker: ");
            message += error;
            return sq_throwerror(vm, message.c_str());
        }

        return 1;
    }

    SQInteger ScriptWorkerHandleUserData::progress(HSQUIRRELVM vm){
        ScriptWorker* worker = _resolve(vm, 0);
        sq_pushfloat(vm, worker ? worker->progress() : 0.0f);

        return 1;
    }

    SQInteger ScriptWorkerHandleUserData::error(HSQUIRRELVM vm){
        ScriptWorker* worker = _resolve(vm, 0);
        if(!worker){
            sq_pushnull(vm);
            return 1;
        }

        const std::string message = worker->getError();
        if(message.empty()) sq_pushnull(vm);
        else sq_pushstring(vm, message.c_str(), -1);

        return 1;
    }

    SQInteger ScriptWorkerHandleUserData::destroy(HSQUIRRELVM vm){
        ScriptWorkerManager* manager = 0;
        ScriptWorkerId id;
        if(readWorkerIdFromUserData(vm, 1, &id) != USER_DATA_GET_SUCCESS){
            return sq_throwerror(vm, "Expected a worker handle.");
        }

        manager = ScriptWorkerNamespace::mManager;
        //Idempotent by construction: a second call finds a generation which no longer matches.
        if(manager) manager->destroy(id);

        return 0;
    }

    SQInteger ScriptWorkerHandleUserData::workerToString(HSQUIRRELVM vm){
        ScriptWorkerId id;
        if(readWorkerIdFromUserData(vm, 1, &id) != USER_DATA_GET_SUCCESS){
            sq_pushstring(vm, "ScriptWorker(invalid)", -1);
            return 1;
        }

        ScriptWorker* worker = ScriptWorkerNamespace::mManager ? ScriptWorkerNamespace::mManager->get(id) : 0;
        const std::string out = "ScriptWorker(" + std::to_string(id.index) + ":" + std::to_string(id.generation) +
                                (worker ? ")" : ", destroyed)");
        sq_pushstring(vm, out.c_str(), -1);

        return 1;
    }

    void ScriptWorkerHandleUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 8);

        ScriptUtils::addFunction(vm, workerToString, "_tostring");

        /**SQFunction
        @name dispatch
        @desc Queue one run of the worker script's run() function, passing it a copy of the given
        table. Only values are copied - a table containing a closure, an instance or userdata is
        rejected with an error, since those cannot cross between vms.
        @param1:Table:The input for this run.
        @returns True if the run was queued, false if the worker is still busy with a previous one.
        */
        ScriptUtils::addFunction(vm, dispatch, "dispatch", 2, ".t");

        /**SQFunction
        @name poll
        @desc Where this worker is up to, as one of _WORKER_IDLE, _WORKER_DISPATCHED,
        _WORKER_RUNNING, _WORKER_READY, _WORKER_FAILED or _WORKER_DESTROYED.
        @returns An integer.
        */
        ScriptUtils::addFunction(vm, poll, "poll");

        /**SQFunction
        @name claim
        @desc Take the result of the last run and return this worker to idle, or null if there is
        nothing to take yet. Claiming after a failed run returns null and clears the error.

        Note that this copies the result out of the worker vm on the main thread, so the size and
        shape of what run() returns is paid for on the frame that claims it. A flat array of
        numbers is cheap; a deeply nested tree of small tables is not. Claim one worker per frame.
        @returns The value run() returned, or null.
        */
        ScriptUtils::addFunction(vm, claim, "claim");

        /**SQFunction
        @name progress
        @desc Whatever the worker last passed to _workerSelf.setProgress, between 0 and 1.
        @returns A float.
        */
        ScriptUtils::addFunction(vm, progress, "progress");

        /**SQFunction
        @name error
        @desc Why the last run failed, or null if it did not. Cleared by claim().
        @returns A string or null.
        */
        ScriptUtils::addFunction(vm, error, "error");

        /**SQFunction
        @name destroy
        @desc Retire this worker and close its vm. Safe to call while a run is in flight - the run
        is asked to cancel and the vm is closed once it stops. The handle is invalid immediately.
        */
        ScriptUtils::addFunction(vm, destroy, "destroy");

        sq_resetobject(&workerDelegateTableObject);
        sq_getstackobj(vm, -1, &workerDelegateTableObject);
        sq_addref(vm, &workerDelegateTableObject);
        sq_pop(vm, 1);
    }
}
