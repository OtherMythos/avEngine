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
            
            bool operator == (const Id &other) const { return _id == other.id(); }
            
            uint64_t id() const { return _id; }
            static const Id INVALID;
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
        
        static std::mutex waitMutex;
        static std::unique_lock<std::mutex> waitLock;
        static std::condition_variable waitCv;
        
        //The number of jobs that have been assigned.
        static uint64_t jobCount;

    public:
        static bool initialise(int numWorkers);
        static bool shutdown();
        static Id dispatchJob(Job *job);
        
        /**
         End a job.
         If the job has already been assigned to a worker thread and is running this function will block and wait for it to finish.
         If the job is in the queue it will be removed and not run at all.
         */
        static void endJob(Id job);

        static bool addWorkerToQueue(Worker *worker);
    };
};
