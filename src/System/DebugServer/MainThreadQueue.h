#ifdef DEBUG_SERVER

#pragma once

#include <functional>
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstdint>

namespace AV{
    /**
    Marshals work from arbitrary threads onto the thread that calls pump().

    The debug server runs its HTTP handlers on their own threads, but engine state
    (the scene graph, ECS, script VM) may only be read safely on the main thread.
    A handler packages its read into a closure and hands it to execute(); the main
    thread runs every queued closure once per frame in pump(). execute() blocks until
    its closure has run, or until a timeout elapses (e.g. the engine is paused in a
    debugger or shutting down).

    This class has no dependency on Ogre, httplib or any engine subsystem, so it can
    be unit tested in isolation.
    */
    class MainThreadQueue{
    public:
        MainThreadQueue() = default;
        ~MainThreadQueue() = default;

        MainThreadQueue(const MainThreadQueue&) = delete;
        MainThreadQueue& operator=(const MainThreadQueue&) = delete;

        /**
        Queue fn to be run on the pump() thread and block until it completes.

        @param fn The work to run on the main thread.
        @param timeoutMs How long to wait for the main thread to service the request.
        @return True if fn ran, false if the request timed out or the queue is shutting down.
                When false, fn is guaranteed never to run after this call returns.
        */
        bool execute(std::function<void()> fn, uint32_t timeoutMs);

        /**
        Run every queued closure. Called once per frame on the main thread.
        */
        void pump();

        /**
        Wake every blocked caller (they receive a false result) and reject future
        execute() calls. Called on the main thread before teardown.
        */
        void shutdown();

    private:
        struct QueryEntry{
            std::function<void()> fn;
            std::mutex mutex;
            std::condition_variable cv;
            bool done = false;
            bool serviced = false;
        };

        std::mutex mQueueMutex;
        std::deque<std::shared_ptr<QueryEntry>> mQueue;
        std::atomic<bool> mShuttingDown{false};
    };
}

#endif
