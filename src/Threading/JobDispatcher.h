#pragma once

#include <thread>
#include <vector>

#include "Job.h"

namespace AV{
    class Worker;

    class JobDispatcher{
    private:
        static std::vector<std::thread*> threads;
        static std::vector<Worker*> workers;

    public:
        static bool initialise(int numWorkers);
        static bool shutdown();
        static void dispatchJob(Job *job);
    };
};
