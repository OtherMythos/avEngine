#include "ScriptWorker.h"

#include "ScriptWorkerJob.h"
#include "ScriptWorkerVM.h"

#include <cassert>

#include "Logger/Log.h"
#include "Scripting/Script/CallbackScript.h"
#include "Scripting/SquirrelDeepCopy.h"
#include "Threading/JobDispatcher.h"

namespace AV{

    namespace{
        //PopulateFunction and ReturnFunction are bare function pointers with nowhere to hang a
        //context, so the worker being run has to be reachable from a global. A file static would
        //race between workers; a thread_local cannot, because a pool thread runs exactly one job
        //at a time and this is only ever set for the duration of that job.
        thread_local ScriptWorker* gRunningWorker = 0;
        thread_local HSQOBJECT gRunResult;
        thread_local bool gRunHasResult = false;

        SQInteger populateRunInput(HSQUIRRELVM vm){
            //The closure and its context table are already on the stack. Push the input, then
            //report the total including the invisible 'this'.
            sq_pushobject(vm, gRunningWorker->getInputObject());
            return 2;
        }

        SQInteger captureRunResult(HSQUIRRELVM vm){
            //callClosure leaves the return value on top and expects this to consume it.
            sq_resetobject(&gRunResult);
            if(SQ_SUCCEEDED(sq_getstackobj(vm, -1, &gRunResult))){
                sq_addref(vm, &gRunResult);
                gRunHasResult = true;
            }
            sq_poptop(vm);

            return 0;
        }
    }

    ScriptWorker::ScriptWorker(ScriptWorkerId id, uint32 seed)
        : mId(id),
          mRng(seed){
        sq_resetobject(&mInputObj);
        sq_resetobject(&mResultObj);
    }

    ScriptWorker::~ScriptWorker(){
        //teardown() must have run on the main thread first. If this fires, a worker was destroyed
        //while the job pool still held the last reference, which would close a vm on a pool thread.
        assert(mVm == 0 && "ScriptWorker destroyed without teardown()");
    }

    bool ScriptWorker::inFlight() const{
        const State s = state();
        return s == State::Dispatched || s == State::Running;
    }

    bool ScriptWorker::prepare(const std::string& resolvedPath, std::string& outError){
        _createVM();

        //prepareRaw rather than prepare, because the res path was already resolved by the caller
        //on the main thread and resolving script:// paths needs a vm with a live call frame.
        if(!mScript->prepareRaw(resolvedPath)){
            outError = getError();
            if(outError.empty()) outError = "could not load worker script at " + resolvedPath;
            return false;
        }

        return _finishPrepare(resolvedPath, outError);
    }

    bool ScriptWorker::prepareFromBuffer(const std::string& scriptSource, const std::string& sourceName, std::string& outError){
        _createVM();

        if(!mScript->prepareFromBuffer(scriptSource, sourceName)){
            outError = getError();
            if(outError.empty()) outError = "could not compile worker script " + sourceName;
            return false;
        }

        return _finishPrepare(sourceName, outError);
    }

    void ScriptWorker::_createVM(){
        assert(mVm == 0);

        mVm = ScriptWorkerVM::create(this);
        mScript = new CallbackScript(mVm);
    }

    bool ScriptWorker::_finishPrepare(const std::string& sourceDescription, std::string& outError){
        //Optional. Gives the script somewhere to build the state it will carry between runs.
        const int setupId = mScript->getCallbackId("setup");
        if(setupId >= 0){
            if(!mScript->call(setupId)){
                outError = getError();
                if(outError.empty()) outError = "setup() failed in worker script " + sourceDescription;
                return false;
            }
        }

        //Resolved here, on the main thread, because getCallbackId builds an Ogre::IdString.
        mEntryClosureId = mScript->getCallbackId("run");
        if(mEntryClosureId < 0){
            outError = "worker script " + sourceDescription + " does not declare a run() function";
            return false;
        }

        //Anything setup() left behind is not a failure of the run which has not happened yet.
        {
            std::unique_lock<std::mutex> lock(mErrorMutex);
            mError.clear();
        }

        return true;
    }

    bool ScriptWorker::dispatch(HSQUIRRELVM mainVm, SQInteger inputIdx, std::string& outError){
        if(!_publishRun(mainVm, inputIdx, outError)) return false;

        //The JobId is dropped deliberately. endJob() either blocks the main thread on a running
        //job or removes a queued one without deleting it, leaving this worker Dispatched forever.
        //Cancellation here is cooperative instead - see requestCancel and _workerSelf.isCancelled.
        JobDispatcher::dispatchJob(new ScriptWorkerJob(shared_from_this()));

        return true;
    }

    bool ScriptWorker::_publishRun(HSQUIRRELVM mainVm, SQInteger inputIdx, std::string& outError){
        //Only the main thread ever moves a worker out of Idle, so this check cannot race.
        if(state() != State::Idle){
            outError = "worker is not idle";
            return false;
        }
        if(!mVm || mEntryClosureId < 0){
            outError = "worker was never prepared";
            return false;
        }

        //The copy happens here, on the main thread, because this is the only moment at which one
        //thread legitimately holds both vms. Doing it from the job would have a pool thread
        //reading the main vm's stack and string table while the main thread is running script in it.
        if(!SquirrelDeepCopy::copyValueStrict(mainVm, mVm, inputIdx, outError)){
            return false;
        }

        _releaseInput();
        sq_getstackobj(mVm, -1, &mInputObj);
        sq_addref(mVm, &mInputObj);
        sq_pop(mVm, 1);

        _releaseResult();

        mProgress.store(0.0f, std::memory_order_relaxed);
        mCancel.store(false, std::memory_order_relaxed);
        {
            std::unique_lock<std::mutex> lock(mErrorMutex);
            mError.clear();
        }

        //Published last, so the pool cannot observe a half prepared worker.
        mState.store(State::Dispatched, std::memory_order_release);

        return true;
    }

    void ScriptWorker::runEntry(){
        mState.store(State::Running, std::memory_order_release);

        gRunningWorker = this;
        gRunHasResult = false;
        sq_resetobject(&gRunResult);

        const bool ok = mScript->call(mEntryClosureId, populateRunInput, captureRunResult);

        if(ok && gRunHasResult){
            mResultObj = gRunResult;
            mHasResult = true;
        }else if(gRunHasResult){
            //The call failed after a value was captured, which should not happen, but the
            //reference has to go back either way.
            sq_release(mVm, &gRunResult);
        }

        gRunningWorker = 0;
        gRunHasResult = false;
        sq_resetobject(&gRunResult);

        _releaseInput();

        if(!ok){
            std::unique_lock<std::mutex> lock(mErrorMutex);
            if(mError.empty()) mError = "worker run() failed";
        }

        //Must be the last thing this function does. Once this store lands the main thread is
        //entitled to touch the vm again.
        mState.store(ok ? State::ResultReady : State::Failed, std::memory_order_release);
    }

    bool ScriptWorker::claim(HSQUIRRELVM mainVm, std::string& outError){
        const State s = state();
        if(s != State::ResultReady && s != State::Failed){
            outError = "worker has no result to claim";
            return false;
        }

        bool success = true;
        if(s == State::ResultReady && mHasResult){
            sq_pushobject(mVm, mResultObj);
            if(!SquirrelDeepCopy::copyValueStrict(mVm, mainVm, -1, outError)){
                success = false;
            }
            sq_pop(mVm, 1);
        }else{
            //A failed run, or a run() which returned nothing.
            sq_pushnull(mainVm);
        }

        _releaseResult();
        mProgress.store(0.0f, std::memory_order_relaxed);
        {
            std::unique_lock<std::mutex> lock(mErrorMutex);
            mError.clear();
        }

        mState.store(State::Idle, std::memory_order_release);

        return success;
    }

    void ScriptWorker::teardown(){
        //Taking the lock closes the last window between a job publishing its final state and the
        //pool deleting the job object.
        {
            std::unique_lock<std::mutex> lock(mErrorMutex);
            mError.clear();
        }

        if(!mVm){
            mState.store(State::Destroyed, std::memory_order_release);
            return;
        }

        assert(!inFlight() && "tearing down a worker while its job is still running");

        _releaseInput();
        _releaseResult();

        //Releases the closures it addref'd against mVm, so it has to go while the vm is open.
        delete mScript;
        mScript = 0;

        sq_setforeignptr(mVm, 0);
        sq_close(mVm);
        mVm = 0;

        mState.store(State::Destroyed, std::memory_order_release);
    }

    void ScriptWorker::recordError(const std::string& message){
        std::unique_lock<std::mutex> lock(mErrorMutex);
        //The first failure of a run is the informative one; squirrel may report more as it unwinds.
        if(mError.empty()) mError = message;
    }

    std::string ScriptWorker::getError() const{
        std::unique_lock<std::mutex> lock(mErrorMutex);
        return mError;
    }

    HSQOBJECT ScriptWorker::getInputObject() const{
        return mInputObj;
    }

    void ScriptWorker::_releaseInput(){
        if(!mVm) return;
        sq_release(mVm, &mInputObj);
        sq_resetobject(&mInputObj);
    }

    void ScriptWorker::_releaseResult(){
        if(!mVm) return;
        if(mHasResult){
            sq_release(mVm, &mResultObj);
            mHasResult = false;
        }
        sq_resetobject(&mResultObj);
    }
}
