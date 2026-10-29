#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "JobId.h"

namespace AV{
    class Job;

    class Worker{
    private:
        std::condition_variable cv;
    	std::mutex mtx;

        //A worker will always be running until the dispatcher shuts down.
        //Both are read by the worker thread and written by the dispatcher's.
        std::atomic<bool> _running;
        //Ready is set to true when this worker is ready to process a request.
        std::atomic<bool> _ready;

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
        void waitForJob(JobId jobId);
        
        JobId getRunningId() { std::lock_guard<std::mutex> lock(mtx); return _jobId; }
        int getWorkerId() { return mWorkerId; };

        std::condition_variable* getConditionVariable();
    };
};
