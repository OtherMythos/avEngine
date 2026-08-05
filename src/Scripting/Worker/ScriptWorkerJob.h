#pragma once

#include <memory>

#include "Threading/Jobs/Job.h"

namespace AV{
    class ScriptWorker;

    /**
    Runs one dispatch of a script worker on the job pool.

    The shared_ptr is what makes destroy() safe to call while a run is queued or in flight: the
    manager lets go of the worker, this job does not, and the object survives until the run is
    over.
    */
    class ScriptWorkerJob : public Job{
    public:
        explicit ScriptWorkerJob(std::shared_ptr<ScriptWorker> worker);
        ~ScriptWorkerJob() = default;

        void process() override;

        /**
        Worker::run calls process() and finish() back to back on the pool thread and then deletes
        the job, so finish() is not a main thread callback and there is nothing useful it can do.
        The result is published into the worker's own vm and the main thread drains it in claim().
        */
        void finish() override {}

    private:
        std::shared_ptr<ScriptWorker> mWorker;
    };
}
