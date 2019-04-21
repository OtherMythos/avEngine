#include "JobDispatcher.h"

#include "Logger/Log.h"

#include "Worker.h"

namespace AV{
    std::vector<std::thread*> JobDispatcher::threads;
    std::vector<Worker*> JobDispatcher::workers;
    std::mutex JobDispatcher::workersMutex;
    std::mutex JobDispatcher::jobMutex;
    std::queue<Worker*> JobDispatcher::workersQueue;
    std::deque<JobDispatcher::JobEntry> JobDispatcher::jobQueue;
    uint64_t JobDispatcher::jobCount = 0;

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
        }

        for(int i = 0; i < threads.size(); i++){
            threads[i]->join();
            delete workers[i];
            delete threads[i];

            AV_INFO("Joined worker thread {}", i);
        }
        
        threads.clear();
        workers.clear();

        return true;
    }

    JobDispatcher::Id JobDispatcher::dispatchJob(Job *job){
        //Lock things up.
        std::unique_lock<std::mutex> workersLock(workersMutex);
        
        //Increment the job count. The value it has now will be the id of this job.
        jobCount++;
        Id jobId(jobCount);
        JobEntry jobEntry(jobId, job);

        //If there is an available worker in the queue.
        if(!workersQueue.empty()){
            Worker *worker = workersQueue.front();
            worker->setJob(jobEntry);
            std::condition_variable* cv;
            cv = worker->getConditionVariable();
            cv->notify_one();

            workersQueue.pop();
        }else{
            //There is no available worker to process the job, so push it into the queue.
            workersLock.unlock();

            std::unique_lock<std::mutex> jobLock(jobMutex);
            jobQueue.push_back(jobEntry);
        }
        
        return jobId;
    }

    bool JobDispatcher::addWorkerToQueue(Worker *worker){
        bool wait = true;
        std::unique_lock<std::mutex> jobLock(jobMutex);

        //If there is a request in the queue make the worker do that.
        //If not push it into the queue to wait until a job comes.
        if(!jobQueue.empty()){
            worker->setJob(jobQueue.front());
            jobQueue.pop_front();
            wait = false;
        }else{
            jobLock.unlock();

            std::unique_lock<std::mutex> workersLock(workersMutex);
            workersQueue.push(worker);
        }

        return wait;
    }

};
