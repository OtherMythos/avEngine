#ifdef DEBUG_SERVER

#include "MainThreadQueue.h"

#include <chrono>

namespace AV{
    bool MainThreadQueue::execute(std::function<void()> fn, uint32_t timeoutMs){
        if(mShuttingDown.load()) return false;

        auto entry = std::make_shared<QueryEntry>();
        entry->fn = std::move(fn);

        {
            std::lock_guard<std::mutex> queueLock(mQueueMutex);
            //Re-check under the lock to close the race with shutdown(), which drains
            //the queue and flips the flag while holding mQueueMutex.
            if(mShuttingDown.load()) return false;
            mQueue.push_back(entry);
        }

        std::unique_lock<std::mutex> entryLock(entry->mutex);
        bool completed = entry->cv.wait_for(entryLock, std::chrono::milliseconds(timeoutMs), [&entry]{
            return entry->done;
        });

        if(!completed){
            //Timed out. Unlink the entry if the pump hasn't already taken it, so it is
            //never run after this call returns. If the pump is concurrently executing it,
            //the shared_ptr keeps the entry alive and the pump's result is simply discarded.
            std::lock_guard<std::mutex> queueLock(mQueueMutex);
            for(auto it = mQueue.begin(); it != mQueue.end(); ++it){
                if(*it == entry){
                    mQueue.erase(it);
                    break;
                }
            }
            return false;
        }

        return entry->serviced;
    }

    void MainThreadQueue::pump(){
        std::deque<std::shared_ptr<QueryEntry>> local;
        {
            //Swap the queue out under the lock so HTTP threads never wait on the frame.
            std::lock_guard<std::mutex> queueLock(mQueueMutex);
            local.swap(mQueue);
        }

        for(auto& entry : local){
            entry->fn();
            {
                std::lock_guard<std::mutex> entryLock(entry->mutex);
                entry->done = true;
                entry->serviced = true;
            }
            entry->cv.notify_one();
        }
    }

    void MainThreadQueue::shutdown(){
        mShuttingDown.store(true);

        std::deque<std::shared_ptr<QueryEntry>> local;
        {
            std::lock_guard<std::mutex> queueLock(mQueueMutex);
            local.swap(mQueue);
        }

        //Wake every blocked caller. They wake with serviced == false, so execute() returns false.
        for(auto& entry : local){
            {
                std::lock_guard<std::mutex> entryLock(entry->mutex);
                entry->done = true;
                entry->serviced = false;
            }
            entry->cv.notify_one();
        }
    }
}

#endif
