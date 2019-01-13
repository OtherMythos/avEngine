#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <mutex>

#include "Job.h"

namespace AV{
    class Worker;

    class JobDispatcher{
    private:
        static std::vector<std::thread*> threads;
        static std::vector<Worker*> workers;
        static std::mutex workersMutex;
        static std::mutex jobMutex;
        static std::queue<Worker*> workersQueue;
        static std::queue<Job*> jobQueue;

    public:
        static bool initialise(int numWorkers);
        static bool shutdown();
        static void dispatchJob(Job *job);

        static bool addWorkerToQueue(Worker *worker);
    };
};
