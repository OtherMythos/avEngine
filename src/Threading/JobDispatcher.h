#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <deque>
#include <condition_variable>

#include "System/EnginePrerequisites.h"
#include "Jobs/Job.h"
#include "JobId.h"

namespace AV{
    class Worker;

    class JobDispatcher{
    public:
        typedef std::pair<JobId, Job*> JobEntry;
        enum class Priority { Normal, High };

    private:
        static std::vector<std::thread*> threads;
        static std::vector<Worker*> workers;
        static std::mutex workersMutex;
        static std::mutex jobMutex;
        static std::queue<Worker*> workersQueue;
        static std::deque<JobEntry> jobQueue;
        static std::deque<JobEntry> highPriorityQueue;

        //The number of jobs that have been assigned.
        static uint64_t jobCount;

        /**
         Create the worker threads, sizing the pool from the NumWorkerThreads setting.
         Called the first time a job is dispatched; does nothing if they already exist.
         */
        static void _startWorkers();

    public:
        static bool shutdown();

        /**
         Hand a job to the pool, creating the worker threads if this is the first job.
         Nothing is threaded until this is called, so a project which never dispatches
         a job never pays for the workers.
         */
        static JobId dispatchJob(Job *job, Priority priority = Priority::Normal);

        /**
         How many worker threads are currently running. Zero until the first job.
         */
        static size_t activeWorkerCount();

        /**
         End a job.
         If the job has already been assigned to a worker thread and is running this function will block and wait for it to finish.
         If the job is in the queue it will be removed and not run at all.
         */
        static void endJob(JobId job);

        static bool addWorkerToQueue(Worker *worker);
    };
};
