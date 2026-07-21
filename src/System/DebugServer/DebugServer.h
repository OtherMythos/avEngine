#ifdef DEBUG_SERVER

#pragma once

#include "MainThreadQueue.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>

#include <rapidjson/document.h>

//Forward declaring httplib::Server keeps the (large) httplib header out of this file;
//it is included only in the .cpp.
namespace httplib{
    class Server;
}

namespace AV{
    class FrameCapture;
    class FrameStore;
    class InputPlayback;

    /**
    A localhost HTTP server exposing a read-only REST API for inspecting the running
    engine. Intended for AI agents (and developers) to query scene state via curl.

    The server listens on its own thread. Because engine state may only be read safely
    on the main thread, each request is marshalled through a MainThreadQueue that the
    main loop drains once per frame via pumpMainThread().

    Guarded behind the DEBUG_SERVER compile definition and enabled at runtime with the
    --debugServer command line flag.
    */
    class DebugServer{
    public:
        DebugServer();
        ~DebugServer();

        /**
        Bind to 127.0.0.1:port and start serving on a background thread.
        A bind failure (e.g. port in use) is logged and leaves the engine running.

        @return True if the server bound and started, false otherwise.
        */
        bool initialise(int port);

        /**
        Drain queued requests on the main thread. Call once per frame.
        A no-op after shutdown().
        */
        void pumpMainThread();

        /**
        Stop the server cleanly: wake blocked requests, stop listening, join the thread.
        Must be called on the main thread before engine teardown.
        */
        void shutdown();

    private:
        void _registerRoutes();

        /**
        Run queryFn on the main thread and write its result as the HTTP response.
        queryFn receives the document to populate and an int status it may set (default 200).
        */
        typedef std::function<void(rapidjson::Document&, int&)> QueryFunction;

        std::unique_ptr<httplib::Server> mServer;
        std::thread mServerThread;
        MainThreadQueue mQueue;
        //Captures the window colour buffer via an Ogre frame listener (it cannot go
        //through mQueue: the pump runs after present, when the Metal drawable is dead).
        std::unique_ptr<FrameCapture> mFrameCapture;
        //Named frame snapshots for before/after comparison. Self-locking, because
        //snapshotting happens on HTTP threads alongside capture.
        std::unique_ptr<FrameStore> mFrameStore;
        //Injects spoofed input; ticked once per frame in pumpMainThread.
        std::unique_ptr<InputPlayback> mInputPlayback;

        int mPort = 0;
        std::atomic<bool> mShutdown{false};
        std::chrono::steady_clock::time_point mStartTime;

        //How long a request waits for the main thread before returning 503.
        static const uint32_t REQUEST_TIMEOUT_MS = 2000;
    };
}

#endif
