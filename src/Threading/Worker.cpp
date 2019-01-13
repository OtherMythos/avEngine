#include "Worker.h"

#include "Logger/Log.h"
#include "Job.h"
#include "JobDispatcher.h"
#include <chrono>

namespace AV{
    Worker::Worker()
    : _ready(false),
    _running(true),
    ulock(std::unique_lock<std::mutex>(mtx)){

    }

    void Worker::run(){
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

    void Worker::setJob(Job *job){
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
