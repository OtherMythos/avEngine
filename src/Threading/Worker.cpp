#include "Worker.h"

#include "Logger/Log.h"
#include "Jobs/Job.h"
#include <chrono>

namespace AV{
    Worker::Worker()
	: _running(true),
    _ready(false){

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

    void Worker::setJob(JobDispatcher::JobEntry job){
        AV_INFO("Starting job with id: {}", job.first.id());
        _jobId = job.first;
        _currentJob = job.second;
        _ready = true;
    }

    void Worker::stop(){
        _running = false;
    }

    std::condition_variable* Worker::getConditionVariable(){
        return &cv;
    }

};
