#include "JobDispatcher.h"

#include "Logger/Log.h"

#include "Worker.h"

namespace AV{
    std::vector<std::thread*> JobDispatcher::threads;
    std::vector<Worker*> JobDispatcher::workers;

    bool JobDispatcher::initialise(int numWorkers){
        AV_INFO("Job Dispatcher creating {} threads", numWorkers);
        std::thread *t = 0;
        Worker* w = 0;
        for(int i = 0; i < numWorkers; i++){
            w = new Worker;
            workers.push_back(w);
            t = new std::thread(&Worker::run, w);
            threads.push_back(t);
        }

        return true;
    }

    bool JobDispatcher::shutdown(){
        for(Worker *w : workers){
            w->stop();
            delete w;
        }

        for(int i = 0; i < threads.size(); i++){
            threads[i]->join();

            AV_INFO("Joined worker thread {}", i);
        }
    }

    void JobDispatcher::dispatchJob(Job *job){

    }

};
