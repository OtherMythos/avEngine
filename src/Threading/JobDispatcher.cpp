#include "JobDispatcher.h"

#include "Logger/Log.h"

#include "System/SystemSetup/SystemSettings.h"
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

    void JobDispatcher::_startWorkers(){
        std::unique_lock<std::mutex> workersLock(workersMutex);
        //Another dispatch may have got here first.
        if(!threads.empty()) return;

        uint8 numWorkers = SystemSettings::getNumWorkerThreads();
        //A pool of nothing would leave the job queued forever, which is worse than a thread.
        if(numWorkers < 1) numWorkers = 1;
        //waitCv is fixed size, and the setup file clamps the setting to match.
        if(numWorkers > waitCv.size()) numWorkers = static_cast<uint8>(waitCv.size());

        AV_INFO("Job Dispatcher creating {} threads", numWorkers);

        waitLock = std::unique_lock<std::mutex>(waitMutex);

        for(uint8 i = 0; i < numWorkers; i++){
            Worker* w = new Worker(i);
            workers.push_back(w);
            threads.push_back(new std::thread(&Worker::run, w));
        }
    }

    size_t JobDispatcher::activeWorkerCount(){
        std::unique_lock<std::mutex> workersLock(workersMutex);
        return threads.size();
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
        //These point at the workers deleted above. Now that the pool is created on demand a
        //dispatch can follow a shutdown, so leaving them behind would be a use after free.
        std::queue<Worker*>().swap(workersQueue);

        //_startWorkers takes this and endJob waits on it. Holding it past shutdown would
        //deadlock the next pool the moment it tried to take it again.
        if(waitLock.owns_lock()) waitLock.unlock();

        //Anything still queued will never run, and nothing else owns it.
        for(JobEntry& e : jobQueue){
            delete e.second;
        }
        jobQueue.clear();

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
        //Both queues are held for the whole check and act. Taking them one at a time lets a
        //worker park itself as idle in the gap after this has already decided there was none,
        //and the job then sits in the queue with nothing left to come looking for it - a worker
        //asleep in Worker::run's wait loop never re-examines the job queue, so that job only
        //moves if some later dispatch happens to find the worker idle.
        //The order matches endJob, so the two cannot deadlock against each other.
        std::unique_lock<std::mutex> workersLock(workersMutex);
        std::unique_lock<std::mutex> jobLock(jobMutex);

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

            return jobId;
        }

        //There is no available worker to process the job, so push it into the queue.
        jobQueue.push_back(jobEntry);

        const bool workersNeeded = threads.empty();
        jobLock.unlock();
        workersLock.unlock();

        //The job goes into the queue before the threads exist, because the first thing a
        //worker does is ask for one. Creating them first would race, and a worker which
        //found the queue empty would park itself and leave this job sitting there.
        if(workersNeeded) _startWorkers();

        return jobId;
    }

    bool JobDispatcher::addWorkerToQueue(Worker *worker){
        bool wait = true;
        //Held together and in the same order as dispatchJob, so that becoming idle and being
        //given a job cannot interleave.
        std::unique_lock<std::mutex> workersLock(workersMutex);
        std::unique_lock<std::mutex> jobLock(jobMutex);

        //If there is a request in the queue make the worker do that.
        //If not push it into the queue to wait until a job comes.
        if(!jobQueue.empty()){
            worker->setJob(jobQueue.front().first, jobQueue.front().second);
            jobQueue.pop_front();
            wait = false;
        }else{
            workersQueue.push(worker);
        }

        jobLock.unlock();
        workersLock.unlock();

        waitCv[worker->getWorkerId()].notify_all();

        return wait;
    }

};
