#ifdef FLIGHT_RECORDER

#pragma once

#include "CaptureWriter.h"
#include "FrameRing.h"
#include "RecorderSettings.h"

#include <string>
#include <cstdint>

namespace AV{

    class RecorderFrameListener;
    class Event;

    /**
    Continuously records the last few seconds of a running game - rendered frames, squirrel
    activity and whatever variables the game asked to watch - into a fixed size circular
    buffer, and writes that buffer to disk on demand.

    The point is post hoc diagnosis: by the time a player notices something wrong, the state
    that caused it is gone. Pressing the capture hotkey dumps everything the recorder was
    holding, along with a description the player types, into a directory an agent can triage
    later.

    Static, like ScriptProfiler, and driven from Base: initialise() at startup,
    notifyFrameBoundary() once per rendered frame, shutdown() before Ogre is torn down.
    Main thread only, apart from the writer thread it owns.
    */
    class FlightRecorder{
    public:
        FlightRecorder() = delete;
        ~FlightRecorder() = delete;

        /**
        Register the frame listener and the script hook and begin recording.
        Call after Ogre and the script vm exist.
        */
        static void initialise(const RecorderSettings& settings);

        /**
        Finish any queued captures, detach from Ogre and the vm, and release the ring.
        Must run while both Ogre and the vm are still alive.
        */
        static void shutdown();

        static bool isEnabled() { return mEnabled; }
        static bool isRunning() { return mEnabled && mRunning; }
        static void start();
        static void stop();

        /**
        Close off the frame just rendered and push it into the ring, then service any capture
        that was requested. Called once per frame by Base, after renderOneFrame.
        */
        static void notifyFrameBoundary();

        /**
        Request a capture.

        The dump itself happens at the next frame boundary, once the full resolution frame has
        been read back, but anything that must be sampled now - notably the live squirrel
        backtrace, which only exists while script code is running - is collected immediately.

        @param description Optional; the hotkey flow leaves this empty and writes it later,
                           once the player has finished typing.
        */
        static void capture(CaptureReason reason, const std::string& description = "");

        /**
        Whether a capture is waiting on its description. The hotkey flow pauses the game
        while this is true.
        */
        static bool awaitingDescription() { return !mAwaitingDescriptionDir.empty(); }

        /**
        Attach a description to the capture that is waiting for one, and stop waiting.
        */
        static void submitDescription(const std::string& description);

        /**
        Abandon waiting for a description. The capture itself is still written.
        */
        static void cancelDescription();

        /**
        Directory of the most recently requested capture, for reporting back to the player
        or to an agent.
        */
        static const std::string& lastCapturePath() { return mLastCapturePath; }

        static size_t framesBuffered();
        static const RecorderSettings& settings() { return mSettings; }

        //Receives the hotkey toggle broadcast by the window.
        static bool debuggerToolsReceiver(const Event& event);

        /**
        Tell the recorder whether anything is currently rendering into the window.

        Reading the colour buffer back is only valid once a workspace has actually drawn to
        it. Until then the swapchain has no drawable, and on Metal the readback dereferences
        it and segfaults inside the driver. A project with a custom compositor creates its
        workspace from script, several frames after the recorder starts, so this cannot be
        assumed - it has to be tracked.
        */
        static void notifyWindowWorkspace(bool present);
        static bool windowHasWorkspace() { return mWindowWorkspaces > 0; }

        //Called by RecorderFrameListener from inside frameRenderingQueued.
        static void _notifyFrameCaptured(CapturedFrame&& frame);
        static void _notifyFullFrame(CapturedFrame&& frame);

    private:
        static void _performCapture();
        static void _populateMetadata(CaptureData& data);
        static std::string _timestamp();

        static bool mEnabled;
        static bool mRunning;
        static RecorderSettings mSettings;

        static FrameRing mRing;
        static CaptureWriter mWriter;
        static RecorderFrameListener* mFrameListener;

        //The downsampled frame read back during the render that just happened, waiting for
        //the frame boundary to pair it with that frame's script data.
        static CapturedFrame mPendingFrame;
        static bool mHavePendingFrame;

        static CapturedFrame mFullFrame;
        static bool mHaveFullFrame;

        static bool mCaptureRequested;
        static CaptureReason mCaptureReason;
        static std::string mCaptureDescription;
        //Collected at request time, while the stack still exists.
        static std::vector<BacktraceFrame> mCaptureBacktrace;
        //Frames waited so far for the full resolution readback.
        static uint32_t mCaptureWaitFrames;

        static std::string mLastCapturePath;
        //Resolved once when the capture is requested, so the directory name and the manifest
        //cannot disagree if the clock ticks over between them.
        static std::string mCaptureTimestamp;
        static std::string mAwaitingDescriptionDir;

        static uint64_t mStartTimeNs;
        //Number of workspaces known to target the window.
        static int mWindowWorkspaces;

        //Pause mask in force before the prompt was shown, restored when it closes.
        static unsigned int mPausedMaskBefore;

        static void _showDescriptionPrompt();
        static void _hideDescriptionPrompt();

        //How long a capture waits for its full resolution frame before writing without one.
        static const uint32_t MAX_CAPTURE_WAIT_FRAMES = 3;
    };
}

#endif
