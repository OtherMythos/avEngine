#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <deque>
#include <condition_variable>

#include "Jobs/Job.h"
#include "JobId.h"

namespace AV{
    class Worker;

    class JobDispatcher{
    public:
        typedef std::pair<JobId, Job*> JobEntry;
        
    private:
        static std::vector<std::thread*> threads;
        static std::vector<Worker*> workers;
        static std::mutex workersMutex;
        static std::mutex jobMutex;
        static std::queue<Worker*> workersQueue;
        static std::deque<JobEntry> jobQueue;

        static std::mutex waitMutex;
        static std::unique_lock<std::mutex> waitLock;
        static std::vector<std::condition_variable> waitCv;

        //The number of jobs that have been assigned.
        static uint64_t jobCount;

    public:
        static bool initialise(int numWorkers);
        static bool shutdown();
        static JobId dispatchJob(Job *job);

        /**
         End a job.
         If the job has already been assigned to a worker thread and is running this function will block and wait for it to finish.
         If the job is in the queue it will be removed and not run at all.
         */
        static void endJob(JobId job);

        static bool addWorkerToQueue(Worker *worker);
    };
};
