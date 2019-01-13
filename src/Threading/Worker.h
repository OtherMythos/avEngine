#pragma once

#include <condition_variable>
#include <mutex>

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

        Job *_currentJob;

    public:
        Worker();
        void run();
        void stop();
        void setJob(Job *job);

        std::condition_variable* getConditionVariable();
    };
};
