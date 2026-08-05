#pragma once

#include <string>
#include <vector>

#include "ScriptWorker.h"

namespace AV{

    /**
    Owns every script worker, and is the only thing script reaches them through.

    Constructed by Base only when ScriptWorkers is enabled in the setup file, so a project which
    does not use the feature pays for nothing - not a vm, not a thread, not even the _worker
    namespace in the main vm.
    */
    class ScriptWorkerManager{
    public:
        ScriptWorkerManager();
        ~ScriptWorkerManager();

        ScriptWorkerManager(const ScriptWorkerManager&) = delete;
        ScriptWorkerManager& operator=(const ScriptWorkerManager&) = delete;

        /**
        Create a worker and load resPath into it. Main thread.
        @return An invalid id on failure, with outError describing why.
        */
        ScriptWorkerId create(const std::string& resPath, std::string& outError);

        /**
        As above, from squirrel source held in memory rather than a file on disk. Main thread.
        @param sourceName What squirrel reports for this script in errors. Never treated as a path.
        */
        ScriptWorkerId createFromBuffer(const std::string& scriptSource, const std::string& sourceName, std::string& outError);

        /**
        The worker for an id, or null if it was destroyed or the id is stale.
        */
        ScriptWorker* get(ScriptWorkerId id) const;

        /**
        Retire a worker. The handle goes stale immediately, but if a run is queued or in flight the
        object is kept alive until it finishes and reaped by a later update().
        */
        void destroy(ScriptWorkerId id);

        /**
        Reap workers whose destroy() had to wait for a run to finish. Main thread, once per frame.
        */
        void update();

        /**
        Cancel and drain every worker, then close their vms. Main thread. Must run before both
        ScriptVM::shutdown and JobDispatcher::shutdown - the first so a late call from script
        finds no manager, the second so queued jobs can still run and the drain terminates.
        */
        void shutdown();

        size_t liveWorkerCount() const;

    private:
        struct Slot{
            uint16 generation = 0;
            ScriptWorkerPtr worker;
        };

        std::vector<Slot> mWorkers;
        //Destroyed while running, waiting for the pool to let go of them.
        std::vector<ScriptWorkerPtr> mPendingDestroy;

        uint16 mNextGeneration = 1;
        uint32 mNextSeed = 1;
        bool mShuttingDown = false;
        bool mWarnedAboutPoolSize = false;

        /**
        Run the checks every create shares and pick a slot, returning an invalid id on refusal.
        The slot is not claimed until _commit, so a script which fails to load leaves no trace.
        */
        ScriptWorkerId _allocate(ScriptWorkerPtr& outWorker, std::string& outError);
        ScriptWorkerId _commit(ScriptWorkerId id, ScriptWorkerPtr worker);

        /**
        Close a worker's vm, or if the pool is still inside it, park the worker somewhere it will
        outlive the process rather than pull the vm out from under a running script.
        */
        void _retire(ScriptWorkerPtr& worker);
    };
}
