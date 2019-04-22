#pragma once

#include <condition_variable>
#include <mutex>

#include "JobId.h"

namespace AV{
    class Job;

    class Worker{
    private:
        std::condition_variable cv;
    	std::mutex mtx;
    	std::unique_lock<std::mutex> ulock;

        //A worker will always be running until the dispatcher shuts down.
        bool _running;
        //Ready is set to true when this worker is ready to process a request.
        bool _ready;

        JobId _jobId;
        Job *_currentJob;

		bool runOnce = false;
        
        int mWorkerId;

    public:
        Worker(int workerId);
        void run();
        void stop();
        void setJob(JobId id, Job* job);
        bool runningJob(JobId jobId);
        
        JobId getRunningId() { return _jobId; }
        int getWorkerId() { return mWorkerId; };

        std::condition_variable* getConditionVariable();
    };
};
