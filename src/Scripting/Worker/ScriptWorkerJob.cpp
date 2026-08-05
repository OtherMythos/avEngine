#include "ScriptWorkerJob.h"

#include "ScriptWorker.h"

namespace AV{

    ScriptWorkerJob::ScriptWorkerJob(std::shared_ptr<ScriptWorker> worker)
        : mWorker(std::move(worker)){

    }

    void ScriptWorkerJob::process(){
        mWorker->runEntry();
    }
}
