#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <deque>

#include "Jobs/Job.h"

namespace AV{
    class Worker;

    class JobDispatcher{
    public:
        //Id of a job.
        //This is to encapsulate the actual id of the job within a private scope.
        struct Id{
            friend JobDispatcher;
        public:
            //0 represents an invalid id.
            Id() : _id(0) {}
            
            uint64_t id() const { return _id; }
        private:
            Id(uint64_t i) : _id(i) {}
            
            uint64_t _id;
        };
        
        typedef std::pair<Id, Job*> JobEntry;
        
    private:
        static std::vector<std::thread*> threads;
        static std::vector<Worker*> workers;
        static std::mutex workersMutex;
        static std::mutex jobMutex;
        static std::queue<Worker*> workersQueue;
        static std::deque<JobEntry> jobQueue;
        
        //The number of jobs that have been assigned.
        static uint64_t jobCount;

    public:
        static bool initialise(int numWorkers);
        static bool shutdown();
        static Id dispatchJob(Job *job);

        static bool addWorkerToQueue(Worker *worker);
    };
};
