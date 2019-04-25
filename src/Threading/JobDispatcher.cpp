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
    
    std::mutex JobDispatcher::waitMutex;
    std::unique_lock<std::mutex> JobDispatcher::waitLock;
    //Creating 4 manually because I'm not sure if this is the solution.
    std::vector<std::condition_variable> JobDispatcher::waitCv(4);
    
    const JobId JobId::INVALID;

    bool JobDispatcher::initialise(int numWorkers){
        AV_INFO("Job Dispatcher creating {} threads", numWorkers);
        std::thread *t = 0;
        Worker* w = 0;
        for(int i = 0; i < numWorkers; i++){
            w = new Worker(i);
            workers.push_back(w);
            t = new std::thread(&Worker::run, w);
            threads.push_back(t);
        }
        
        waitLock = std::unique_lock<std::mutex>(waitMutex);

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
    
    void JobDispatcher::endJob(JobId job){
        if(job == JobId::INVALID) return;
        //Check if a worker contains this job.
        //Otherwise check the job queue.
        //If nothing could be found then return straight away because there is no job with that id.
        
        //Both need to be locked, as otherwise a race condition is possible.
        std::unique_lock<std::mutex> workersLock(workersMutex);
        std::unique_lock<std::mutex> jobLock(jobMutex);
        
        Worker* targetW = 0;
        for(Worker* w : workers){
            if(w->runningJob(job)){
                targetW = w;
                break;
            }
        }
        
        if(targetW != 0){
            //The job is being run by a worker. So I should wait for that worker.
            AV_INFO("Job {} being run by worker.", job.id());
            
            //The mutexes can be unlocked, so they're only for the searching part.
            //If they're not unlocked the system will reach a deadlock in the addWorkerToQueue function.
            workersLock.unlock();
            jobLock.unlock();
            
            waitCv[targetW->getWorkerId()].wait(waitLock);
            AV_INFO("Finished waiting.");
            
            return;
        }else{
            //The job is not being run by a worker.
            //Search the queue now.
            AV_INFO("Job {} not being run by worker.", job.id());
            
            auto jit = jobQueue.begin();
            while(jit != jobQueue.end()){
                if((*jit).first == job) break;
                jit++;
            }
            if(jit != jobQueue.end()){
                AV_INFO("Job {} found in queue", job.id());
                jobQueue.erase(jit);
                AV_INFO("Job {} removed from queue", job.id());
                return;
            }else{
                AV_INFO("Job {} not found in queue", job.id());
            }
        }
    }

    JobId JobDispatcher::dispatchJob(Job *job){
        //Lock things up.
        std::unique_lock<std::mutex> workersLock(workersMutex);
        
        //Increment the job count. The value it has now will be the id of this job.
        jobCount++;
        JobId jobId(jobCount);
        JobEntry jobEntry(jobId, job);

        //If there is an available worker in the queue.
        if(!workersQueue.empty()){
            Worker *worker = workersQueue.front();
            worker->setJob(jobEntry.first, jobEntry.second);
            std::condition_variable* cv;
            cv = worker->getConditionVariable();
            cv->notify_one();
            AV_INFO("Notifying");
            
            AV_INFO("Job {} going straight to worker.", jobCount);

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
            worker->setJob(jobQueue.front().first, jobQueue.front().second);
            jobQueue.pop_front();
            wait = false;
        }else{
            jobLock.unlock();

            std::unique_lock<std::mutex> workersLock(workersMutex);
            workersQueue.push(worker);
        }
        waitCv[worker->getWorkerId()].notify_all();

        return wait;
    }

};
