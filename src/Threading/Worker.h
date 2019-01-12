#pragma once

#include <condition_variable>
#include <mutex>

namespace AV{
    class Worker{
        std::condition_variable cv;
    	std::mutex mtx;
    	std::unique_lock<std::mutex> ulock;

        //A worker will always be running until the dispatcher shuts down.
        bool running;
        //Ready is set to true when this worker is ready to process a request.
        bool ready;

    public:
        Worker();
        void run();
        void stop() { running = false; }

    };
};
