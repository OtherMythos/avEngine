#pragma once

#include <squirrel.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <random>
#include <string>

#include "System/EnginePrerequisites.h"

namespace AV{
    class CallbackScript;

    /**
    Identifies a worker to script. The generation is bumped when a slot is reused, so a handle
    held across a destroy() refers to nothing rather than to whatever took its place.
    A generation of 0 is never issued and means invalid.
    */
    struct ScriptWorkerId{
        uint16 index = 0;
        uint16 generation = 0;

        bool valid() const { return generation != 0; }
        bool operator==(const ScriptWorkerId& o) const { return index == o.index && generation == o.generation; }
    };

    /**
    A squirrel script running on a worker thread, in its own reduced virtual machine.

    The script file is loaded once and the vm is kept alive, so anything the script stores in its
    table survives from one run to the next. That is what makes this usable for a generator whose
    output depends on everything it has already produced, rather than only for pure functions.

    ============================ THE THREADING RULE ============================
    The worker vm is only ever touched by one thread at a time, and which thread that is, is
    determined solely by mState:

        Idle / ResultReady / Failed   the main thread owns the vm
        Running                       the pool thread owns the vm
        Dispatched                    the handover - nobody may touch it

    Every transition has exactly one writer, and the writer of a transition is the reader of the
    next one, so a single release/acquire atomic orders the whole vm. There is deliberately no
    mutex around any squirrel call; if you find yourself wanting one, the state machine has been
    broken somewhere.

    Consequently the input is copied into this vm at dispatch time and the result is copied out at
    claim time, both on the main thread. The pool thread never sees the main vm, and the main
    thread never touches this vm while a run is in flight.
    ===========================================================================
    */
    class ScriptWorker : public std::enable_shared_from_this<ScriptWorker>{
    public:
        enum class State : uint8{
            Idle = 0,
            Dispatched = 1,
            Running = 2,
            ResultReady = 3,
            Failed = 4,
            Destroyed = 5
        };

        ScriptWorker(ScriptWorkerId id, uint32 seed);
        ~ScriptWorker();

        ScriptWorker(const ScriptWorker&) = delete;
        ScriptWorker& operator=(const ScriptWorker&) = delete;

        /**
        Create the vm and load the script. Main thread, once, before anything else.
        Calls the script's setup() closure if it declares one, and requires a run() closure.
        */
        bool prepare(const std::string& resolvedPath, std::string& outError);

        /**
        As above, from squirrel source held in memory rather than a file on disk.
        @param sourceName What squirrel reports for this script in errors. Never treated as a path.
        */
        bool prepareFromBuffer(const std::string& scriptSource, const std::string& sourceName, std::string& outError);

        /**
        Copy the value at inputIdx in mainVm into this worker and queue a run.
        Main thread. Returns false without queueing anything if the worker is not idle or if the
        input holds a value which cannot cross between vms, in which case outError says why.
        */
        bool dispatch(HSQUIRRELVM mainVm, SQInteger inputIdx, std::string& outError);

        /**
        Run the entry closure. This is the only function a pool thread may call.
        */
        void runEntry();

        /**
        Push the result of the last run onto mainVm and return the worker to idle. Main thread.
        Pushes null if the run failed. Returns false, pushing nothing, if there is no result to
        take or if the result could not be copied out.
        */
        bool claim(HSQUIRRELVM mainVm, std::string& outError);

        /**
        Release everything held in the vm and close it. Main thread, and only once the worker is
        no longer Dispatched or Running.
        */
        void teardown();

        ScriptWorkerId getId() const { return mId; }
        State state() const { return mState.load(std::memory_order_acquire); }
        bool inFlight() const;

        float progress() const { return mProgress.load(std::memory_order_relaxed); }
        void setProgress(float f) { mProgress.store(f, std::memory_order_relaxed); }

        bool cancelRequested() const { return mCancel.load(std::memory_order_relaxed); }
        void requestCancel() { mCancel.store(true, std::memory_order_relaxed); }

        /**
        Record why the last run failed. Called from the worker vm's error handler, so it may run
        on either thread.
        */
        void recordError(const std::string& message);
        std::string getError() const;

        //Worker thread only, and only for the duration of a run.
        std::mt19937& rng() { return mRng; }

        HSQUIRRELVM getVM() const { return mVm; }
        //The copy of this run's input, living in this worker's vm. Only meaningful during a run.
        HSQOBJECT getInputObject() const;

    private:
        ScriptWorkerId mId;

        HSQUIRRELVM mVm = 0;
        CallbackScript* mScript = 0;
        //Resolved once, on the main thread, because getCallbackId builds an Ogre::IdString and
        //the worker thread must never touch Ogre. Do not add a call-by-name path here.
        int mEntryClosureId = -1;

        //Both live in mVm.
        HSQOBJECT mInputObj;
        HSQOBJECT mResultObj;
        bool mHasResult = false;

        std::atomic<State> mState{State::Idle};
        std::atomic<float> mProgress{0.0f};
        std::atomic<bool> mCancel{false};

        //Guards mError, and closes the window between a job publishing its final state and the
        //pool deleting the job object.
        mutable std::mutex mErrorMutex;
        std::string mError;

        std::mt19937 mRng;

        //Shared by both prepare paths, either side of the one step which differs.
        void _createVM();
        bool _finishPrepare(const std::string& sourceDescription, std::string& outError);

        /**
        The half of dispatch which does not involve the job pool: copy the input into this vm and
        publish Dispatched. Split out so the ordering the threading rule depends on - the input is
        in place before any job exists to observe it - is structural rather than a comment.
        */
        bool _publishRun(HSQUIRRELVM mainVm, SQInteger inputIdx, std::string& outError);

        void _releaseInput();
        void _releaseResult();
    };

    typedef std::shared_ptr<ScriptWorker> ScriptWorkerPtr;
}
