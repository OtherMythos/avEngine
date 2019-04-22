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
		if(!runOnce) {
			//This was moved here because vc2017 complained because the lock wasn't created by the actual thread that would run this worker. The constructor is run by the main thread.
			//This lead to problems with the condition variable.
			//Instead it's created here, which is where run is called.
			ulock = std::unique_lock<std::mutex>(mtx);
			runOnce = true;
		}
        while(_running){
            //ready will be set to true when a job has been provided.
            if(_ready){
                //Execute the job.
                _ready = false;
                _currentJob->process();
                _currentJob->finish();
                AV_INFO("Finishing job with id: {}", _jobId.id());
                _jobId = JobId::INVALID;
                delete _currentJob;
            }

            if(JobDispatcher::addWorkerToQueue(this)){
                while(!_ready && _running){
                    if (cv.wait_for(ulock, std::chrono::seconds(1)) == std::cv_status::timeout) {
                    }
                }
            }
        }
    }
    
    bool Worker::runningJob(JobId jobId){
        if(_jobId == jobId) return true;
        return false;
    }

    void Worker::setJob(JobId id, Job* job){
        AV_INFO("Starting job with id: {}", id.id());
        _jobId = id;
        _currentJob = job;
        _ready = true;
    }

    void Worker::stop(){
        _running = false;
    }

    std::condition_variable* Worker::getConditionVariable(){
        return &cv;
    }

};
