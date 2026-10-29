#include "Worker.h"

#include "Logger/Log.h"
#include "Jobs/Job.h"
#include <chrono>

#include "JobDispatcher.h"

namespace AV{
    Worker::Worker(int workerId)
	:_running(true),
    _ready(false),
    mWorkerId(workerId){

    }

    void Worker::run(){
        JobDispatcher::addWorkerToQueue(this);
        while(true){
            Job* job;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]{ return _ready.load() || !_running.load(); });
                if(!_ready) return;
                job = _currentJob;
                _ready = false;
            }
            job->process();
            job->finish();
            delete job;
            {
                std::lock_guard<std::mutex> lock(mtx);
                _jobId = JobId::INVALID;
            }
            cv.notify_all();
            if(!_running) return;
            JobDispatcher::addWorkerToQueue(this);
        }
    }

    bool Worker::runningJob(JobId jobId){
        std::lock_guard<std::mutex> lock(mtx);
        return _jobId == jobId;
    }

    void Worker::waitForJob(JobId jobId){
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this, jobId]{ return !(_jobId == jobId); });
    }

    void Worker::setJob(JobId id, Job* job){
        {
            //Publish under the wait mutex so a notification cannot land between
            //the predicate check and sleeping, stalling audio until a timeout.
            std::lock_guard<std::mutex> lock(mtx);
            _jobId = id;
            _currentJob = job;
            _ready = true;
        }
        cv.notify_all();
    }

    void Worker::stop(){
        {
            //Use the same mutex as the wait predicate.
            std::unique_lock<std::mutex> lock(mtx);
            _running = false;
        }
        cv.notify_all();
    }

    std::condition_variable* Worker::getConditionVariable(){
        return &cv;
    }

};
