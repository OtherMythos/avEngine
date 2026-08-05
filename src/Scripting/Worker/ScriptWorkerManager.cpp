#include "ScriptWorkerManager.h"

#include <chrono>
#include <thread>

#include "Logger/Log.h"
#include "Scripting/ScriptNamespace/ScriptWorkerNamespace.h"
#include "System/SystemSetup/SystemSettings.h"
#include "System/Util/PathUtils.h"

namespace AV{

    namespace{
        //How long shutdown will wait for a run which is ignoring isCancelled().
        const int SHUTDOWN_DRAIN_TIMEOUT_MS = 5000;

        //Workers which were still running when the drain timed out. Their vms cannot be closed -
        //a pool thread is inside one - and their objects cannot be freed, since the pool thread is
        //also using those. Holding a reference here keeps both alive until the process exits,
        //which is the only thing that is actually safe to do at that point. Deliberately never
        //cleared; if this ever holds anything, something in a worker script ignored isCancelled().
        std::vector<ScriptWorkerPtr> gAbandonedWorkers;
    }

    void ScriptWorkerManager::_retire(ScriptWorkerPtr& worker){
        if(!worker) return;

        if(worker->inFlight()){
            gAbandonedWorkers.push_back(worker);
            return;
        }

        worker->teardown();
    }

    ScriptWorkerManager::ScriptWorkerManager(){
        //How script reaches the manager. Same shape as the other namespaces which need a
        //subsystem pointer, and avoids touching BaseSingleton's positional constructor.
        ScriptWorkerNamespace::mManager = this;
    }

    ScriptWorkerManager::~ScriptWorkerManager(){
        //shutdown() should already have run, but a manager destroyed without it must still not
        //leave script pointing at freed memory.
        if(ScriptWorkerNamespace::mManager == this) ScriptWorkerNamespace::mManager = 0;
    }

    size_t ScriptWorkerManager::liveWorkerCount() const{
        size_t count = 0;
        for(const Slot& s : mWorkers){
            if(s.generation != 0) count++;
        }
        return count + mPendingDestroy.size();
    }

    ScriptWorkerId ScriptWorkerManager::create(const std::string& resPath, std::string& outError){
        ScriptWorkerPtr worker;
        const ScriptWorkerId id = _allocate(worker, outError);
        if(!id.valid()) return id;

        std::string resolvedPath;
        //Resolved on the main thread. The worker's own vm has no call frame to resolve script://
        //against at this point, and reading the main vm from the worker thread later would race it.
        formatResToPath(resPath, resolvedPath);

        if(!worker->prepare(resolvedPath, outError)){
            worker->teardown();
            return ScriptWorkerId();
        }

        return _commit(id, worker);
    }

    ScriptWorkerId ScriptWorkerManager::createFromBuffer(const std::string& scriptSource, const std::string& sourceName, std::string& outError){
        ScriptWorkerPtr worker;
        const ScriptWorkerId id = _allocate(worker, outError);
        if(!id.valid()) return id;

        if(!worker->prepareFromBuffer(scriptSource, sourceName, outError)){
            worker->teardown();
            return ScriptWorkerId();
        }

        return _commit(id, worker);
    }

    ScriptWorkerId ScriptWorkerManager::_allocate(ScriptWorkerPtr& outWorker, std::string& outError){
        if(mShuttingDown){
            outError = "the engine is shutting down";
            return ScriptWorkerId();
        }

        const size_t maxWorkers = SystemSettings::getMaxScriptWorkers();
        if(liveWorkerCount() >= maxWorkers){
            //A cap rather than an unbounded grow, because each worker is a whole squirrel vm and
            //a script creating them in a loop should get an error rather than exhaust memory.
            outError = "cannot create more than " + std::to_string(maxWorkers) +
                       " script workers - see MaxWorkers in the ScriptWorkers setup entry";
            return ScriptWorkerId();
        }

        if(!mWarnedAboutPoolSize && liveWorkerCount() >= SystemSettings::getNumWorkerThreads()){
            //Not an error. Runs simply queue behind one another on the shared job pool.
            AV_WARN("There are now as many script workers as job pool threads ({}). Dispatches will queue.", SystemSettings::getNumWorkerThreads());
            mWarnedAboutPoolSize = true;
        }

        size_t index = mWorkers.size();
        for(size_t i = 0; i < mWorkers.size(); i++){
            if(mWorkers[i].generation == 0){
                index = i;
                break;
            }
        }
        if(index == mWorkers.size()) mWorkers.push_back(Slot());

        //Never issue generation 0, that being the marker for an empty slot and an invalid handle.
        if(mNextGeneration == 0) mNextGeneration = 1;

        ScriptWorkerId id;
        id.index = static_cast<uint16>(index);
        id.generation = mNextGeneration++;

        outWorker = std::make_shared<ScriptWorker>(id, mNextSeed++);

        return id;
    }

    ScriptWorkerId ScriptWorkerManager::_commit(ScriptWorkerId id, ScriptWorkerPtr worker){
        //The slot is only claimed once the script loaded, so a failed create leaves no trace and
        //the next one reuses the index.
        mWorkers[id.index].generation = id.generation;
        mWorkers[id.index].worker = worker;

        return id;
    }

    ScriptWorker* ScriptWorkerManager::get(ScriptWorkerId id) const{
        if(!id.valid()) return 0;
        if(id.index >= mWorkers.size()) return 0;

        const Slot& slot = mWorkers[id.index];
        if(slot.generation != id.generation) return 0;

        return slot.worker.get();
    }

    void ScriptWorkerManager::destroy(ScriptWorkerId id){
        if(!id.valid() || id.index >= mWorkers.size()) return;

        Slot& slot = mWorkers[id.index];
        if(slot.generation != id.generation) return;

        ScriptWorkerPtr worker = slot.worker;

        //Freed first so the handle goes stale immediately, whether or not the vm can be closed yet.
        slot.generation = 0;
        slot.worker.reset();

        if(!worker) return;

        worker->requestCancel();

        if(worker->inFlight()){
            //A pool thread is inside this worker's vm, so closing it now would be a use after
            //free. The job holds its own reference; update() closes the vm once it lets go.
            mPendingDestroy.push_back(worker);
            return;
        }

        worker->teardown();
    }

    void ScriptWorkerManager::update(){
        if(mPendingDestroy.empty()) return;

        for(size_t i = 0; i < mPendingDestroy.size(); ){
            ScriptWorkerPtr& worker = mPendingDestroy[i];
            if(worker->inFlight()){
                i++;
                continue;
            }

            worker->teardown();
            mPendingDestroy.erase(mPendingDestroy.begin() + i);
        }
    }

    void ScriptWorkerManager::shutdown(){
        if(mShuttingDown) return;
        mShuttingDown = true;

        for(Slot& s : mWorkers){
            if(s.worker) s.worker->requestCancel();
        }
        for(ScriptWorkerPtr& w : mPendingDestroy){
            w->requestCancel();
        }

        //Wait for the pool to give the vms back. The job pool is still alive at this point, which
        //is why this has to run before JobDispatcher::shutdown - a queued job that never runs
        //would leave a worker Dispatched and spin this out to its timeout.
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(SHUTDOWN_DRAIN_TIMEOUT_MS);
        while(true){
            bool anyInFlight = false;
            for(Slot& s : mWorkers){
                if(s.worker && s.worker->inFlight()) anyInFlight = true;
            }
            for(ScriptWorkerPtr& w : mPendingDestroy){
                if(w->inFlight()) anyInFlight = true;
            }

            if(!anyInFlight) break;
            if(std::chrono::steady_clock::now() >= deadline){
                AV_ERROR("A script worker did not finish within {}ms of shutdown. Its vm will be leaked rather than closed underneath a running script.", SHUTDOWN_DRAIN_TIMEOUT_MS);
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        //Every sq_close happens here, on the main thread, and only for workers the pool has
        //finished with.
        for(Slot& s : mWorkers){
            if(s.worker) _retire(s.worker);
            s.worker.reset();
            s.generation = 0;
        }
        for(ScriptWorkerPtr& w : mPendingDestroy){
            _retire(w);
        }

        mWorkers.clear();
        mPendingDestroy.clear();

        if(ScriptWorkerNamespace::mManager == this) ScriptWorkerNamespace::mManager = 0;
    }
}
