#ifdef FLIGHT_RECORDER

#pragma once

#include "FrameRing.h"
#include "ScriptEventRing.h"
#include "ScriptTrace.h"

#include <atomic>
#include <string>
#include <thread>
#include <vector>
#include <cstdint>

namespace AV{

    /**
    Why a capture was taken. Recorded in the manifest so a triaging agent knows whether the
    backtrace is meaningful (it only is when script code was running).
    */
    enum class CaptureReason{
        Hotkey,
        Script,
        ScriptError,
        DebugServer
    };

    const char* captureReasonName(CaptureReason reason);

    /**
    Everything one capture needs to write itself out, owned outright by the writer.

    The recorder moves its ring into here and immediately allocates a fresh one, so gameplay
    resumes at once and the pixels are never copied.
    */
    struct CaptureData{
        std::string directory;
        CaptureReason reason = CaptureReason::Hotkey;
        std::string description;

        std::vector<FrameRecord> frames;
        //The capture frame at full resolution. Empty if the readback failed.
        CapturedFrame fullFrame;

        //Interned function table the events index into.
        std::vector<TracedFunction> functions;
        //Events, flattened in chronological order across every frame in the capture.
        std::vector<ScriptEvent> events;
        //Absolute index of events[0], so per frame slices can be resolved against it.
        uint64_t eventsBegin = 0;
        std::vector<BacktraceFrame> backtrace;

        //Metadata gathered on the main thread, where it is safe to read.
        std::string engineVersion;
        std::string gitHash;
        std::string project;
        std::string renderSystem;
        std::string commandLine;
        std::string timestamp;
        uint32_t windowWidth = 0;
        uint32_t windowHeight = 0;
        double uptimeSeconds = 0.0;
        uint32_t ringCapacity = 0;
        uint32_t everyNthFrame = 0;
        std::string logPath;
        //Json produced by the debug server's inspectors, empty when it is not compiled in.
        std::string sceneJson;
        std::string guiJson;
    };

    /**
    Serialises captures to disk on a background thread.

    Encoding a hundred pngs takes seconds, which is far too long to spend on the main thread,
    so a capture is handed over wholesale and written while the game carries on. Captures are
    queued, so pressing the hotkey twice in quick succession writes both.
    */
    class CaptureWriter{
    public:
        CaptureWriter() = default;
        ~CaptureWriter();

        /**
        Start the writer thread.
        */
        void initialise();

        /**
        Hand over a capture. Returns immediately; the write happens on the writer thread.
        */
        void submit(CaptureData&& capture);

        /**
        Write a description into an already submitted capture's directory. Safe to call after
        the capture was submitted, whether or not it has finished writing - the directory is
        created by the main thread before submission.
        */
        static bool writeDescription(const std::string& directory, const std::string& description);

        /**
        Stop the thread, waiting for queued captures to finish so nothing is lost on exit.
        */
        void shutdown();

        //Whether any capture is queued or being written.
        bool busy() const;

    private:
        void _threadMain();
        static void _write(const CaptureData& capture);
        static std::string _buildManifest(const CaptureData& capture);
        static std::string _buildTimeline(const CaptureData& capture);
        static std::string _buildScript(const CaptureData& capture);
        static void _writeLogTail(const CaptureData& capture);

        std::thread mThread;
        std::atomic<bool> mRunning{false};
        std::atomic<int> mOutstanding{0};

        //Defined in the cpp so the header need not pull in mutex/condition_variable/deque.
        struct Queue;
        Queue* mQueue = nullptr;
    };
}

#endif
