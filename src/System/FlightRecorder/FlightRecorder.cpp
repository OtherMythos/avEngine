#ifdef FLIGHT_RECORDER

#include "FlightRecorder.h"

#include "RecorderFrameListener.h"
#include "ScriptTrace.h"

#include "System/BaseSingleton.h"
#include "System/EnginePrerequisites.h"
#include "System/SystemSetup/SystemSettings.h"
#include "Event/Events/DebuggerToolEvent.h"
#include "Scripting/ScriptVM.h"
#include "Window/Window.h"
#include "Gui/GuiManager.h"
#include "System/Pause/PauseState.h"
#include "Logger/Log.h"
#include "git_version.h"

#ifdef DEBUG_SERVER
    #include "System/DebugServer/Inspection/SceneInspector.h"
    #include "System/DebugServer/Inspection/GuiInspector.h"
    #include <rapidjson/document.h>
    #include <rapidjson/writer.h>
    #include <rapidjson/stringbuffer.h>
#endif

#include <chrono>
#include <ctime>
#include <filesystem>

namespace AV{

    bool FlightRecorder::mEnabled = false;
    bool FlightRecorder::mRunning = false;
    RecorderSettings FlightRecorder::mSettings;

    FrameRing FlightRecorder::mRing;
    CaptureWriter FlightRecorder::mWriter;
    RecorderFrameListener* FlightRecorder::mFrameListener = nullptr;

    CapturedFrame FlightRecorder::mPendingFrame;
    bool FlightRecorder::mHavePendingFrame = false;
    CapturedFrame FlightRecorder::mFullFrame;
    bool FlightRecorder::mHaveFullFrame = false;

    bool FlightRecorder::mCaptureRequested = false;
    CaptureReason FlightRecorder::mCaptureReason = CaptureReason::Hotkey;
    std::string FlightRecorder::mCaptureDescription;
    std::vector<BacktraceFrame> FlightRecorder::mCaptureBacktrace;
    uint32_t FlightRecorder::mCaptureWaitFrames = 0;

    std::string FlightRecorder::mLastCapturePath;
    std::string FlightRecorder::mCaptureTimestamp;
    std::string FlightRecorder::mAwaitingDescriptionDir;

    uint64_t FlightRecorder::mStartTimeNs = 0;
    int FlightRecorder::mWindowWorkspaces = 0;
    unsigned int FlightRecorder::mPausedMaskBefore = 0;

    static uint64_t nowNs(){
        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    void FlightRecorder::initialise(const RecorderSettings& settings){
        if(mEnabled) return;

        mSettings = settings;
        mStartTimeNs = nowNs();

        mRing.configure(mSettings.ringFrames, mSettings.maxFrameBytes);

        mFrameListener = new RecorderFrameListener();
        mFrameListener->initialise(mSettings.captureWidth, mSettings.captureHeight, mSettings.everyNthFrame);

        mWriter.initialise();

        mEnabled = true;
        mRunning = true;

        //Set last: the frame listener and hook both check isRunning().
        ScriptTrace::initialise(ScriptVM::getVMForRecorder(), mSettings);

        AV_INFO("Flight recorder recording {} frames at {}x{} (every {} frame(s)).",
            mSettings.ringFrames, mSettings.captureWidth, mSettings.captureHeight, mSettings.everyNthFrame);
    }

    void FlightRecorder::shutdown(){
        if(!mEnabled) return;

        mRunning = false;

        //Any capture still pending is written now, while the metadata it needs is still
        //readable, rather than being lost to a clean exit.
        if(mCaptureRequested) _performCapture();

        ScriptTrace::shutdown();

        if(mFrameListener){
            mFrameListener->shutdown();
            delete mFrameListener;
            mFrameListener = nullptr;
        }

        //Blocks until queued captures are on disk.
        mWriter.shutdown();

        mRing.clear();
        mPendingFrame = CapturedFrame();
        mFullFrame = CapturedFrame();
        mHavePendingFrame = false;
        mHaveFullFrame = false;
        mEnabled = false;

        AV_INFO("Flight recorder shut down.");
    }

    void FlightRecorder::start(){
        if(mEnabled) mRunning = true;
        ScriptTrace::start();
    }

    void FlightRecorder::stop(){
        mRunning = false;
        ScriptTrace::stop();
    }

    void FlightRecorder::notifyWindowWorkspace(bool present){
        if(present){
            mWindowWorkspaces++;
        }else if(mWindowWorkspaces > 0){
            mWindowWorkspaces--;
        }
    }

    size_t FlightRecorder::framesBuffered(){
        return mRing.size();
    }

    void FlightRecorder::_notifyFrameCaptured(CapturedFrame&& frame){
        mPendingFrame = std::move(frame);
        mHavePendingFrame = true;
    }

    void FlightRecorder::_notifyFullFrame(CapturedFrame&& frame){
        mFullFrame = std::move(frame);
        mHaveFullFrame = true;
    }

    void FlightRecorder::notifyFrameBoundary(){
        if(!mEnabled) return;

        if(mRunning && mHavePendingFrame){
            FrameRecord record;
            record.frameNumber = mPendingFrame.frameNumber;
            record.timeMs = static_cast<double>(nowNs() - mStartTimeNs) / 1000000.0;

            const PerformanceStats& stats = BaseSingleton::getPerformanceStats();
            record.frameTimeMs = stats.frameTime;
            record.fps = stats.fps;
            record.dHash = ImageOps::dHash(mPendingFrame);

            //Pairs the frame just rendered with the script activity that produced it.
            ScriptTrace::closeFrame(record);

            record.frame = std::move(mPendingFrame);
            mHavePendingFrame = false;

            mRing.push(std::move(record));
        }

        if(!mCaptureRequested) return;

        mCaptureWaitFrames++;
        //Wait for the full resolution readback, but not indefinitely: if the render target
        //cannot be read the rest of the capture is still worth having.
        if(!mHaveFullFrame && mCaptureWaitFrames < MAX_CAPTURE_WAIT_FRAMES) return;

        _performCapture();
    }

    void FlightRecorder::capture(CaptureReason reason, const std::string& description){
        if(!mEnabled){
            AV_WARN("Flight recorder capture requested, but the recorder is not enabled.");
            return;
        }
        if(mCaptureRequested) return;

        mCaptureRequested = true;
        mCaptureReason = reason;
        mCaptureDescription = description;
        mCaptureWaitFrames = 0;
        mHaveFullFrame = false;

        //Must be sampled here rather than at the frame boundary: a live squirrel stack only
        //exists while script code is running, which for a script triggered capture is now.
        mCaptureBacktrace = ScriptTrace::buildBacktrace();

        mCaptureTimestamp = _timestamp();
        mLastCapturePath = (std::filesystem::path(SystemSettings::getUserDirectoryPath()) / "captures" / mCaptureTimestamp).string();

        if(mFrameListener) mFrameListener->requestFullFrame();
    }

    void FlightRecorder::_performCapture(){
        CaptureData data;
        data.directory = mLastCapturePath;
        data.reason = mCaptureReason;
        data.description = mCaptureDescription;
        data.backtrace = std::move(mCaptureBacktrace);

        //Moves the pixels out and leaves a fresh ring behind, so recording resumes at once
        //and nothing is copied.
        data.frames = mRing.takeAll();
        if(mHaveFullFrame) data.fullFrame = std::move(mFullFrame);

        data.functions = ScriptTrace::functions();
        if(!data.frames.empty()){
            const uint64_t begin = data.frames.front().scriptEventBegin;
            const uint64_t end = data.frames.back().scriptEventEnd;
            data.events = ScriptTrace::sliceEvents(begin, end);
            //slice() clamps to what the ring still holds, so report where it actually starts.
            data.eventsBegin = end >= data.events.size() ? end - data.events.size() : 0;
        }

        _populateMetadata(data);

        //Created here, on the main thread, so the description can be written into it before
        //the writer has finished with the frames.
        std::error_code ec;
        std::filesystem::create_directories(data.directory, ec);

        const bool wantsDescription = mCaptureDescription.empty() && mCaptureReason == CaptureReason::Hotkey;

        mWriter.submit(std::move(data));

        mCaptureRequested = false;
        mCaptureDescription.clear();
        mCaptureBacktrace.clear();
        mHaveFullFrame = false;
        mCaptureWaitFrames = 0;

        if(wantsDescription){
            mAwaitingDescriptionDir = mLastCapturePath;
            _showDescriptionPrompt();
        }

        AV_INFO("Flight recorder capture taken: {}", mLastCapturePath);
    }

    void FlightRecorder::submitDescription(const std::string& description){
        if(mAwaitingDescriptionDir.empty()) return;
        CaptureWriter::writeDescription(mAwaitingDescriptionDir, description);
        mAwaitingDescriptionDir.clear();
        _hideDescriptionPrompt();
        AV_INFO("Flight recorder capture description saved.");
    }

    void FlightRecorder::cancelDescription(){
        if(mAwaitingDescriptionDir.empty()) return;
        //The capture itself is still written; only the description is skipped.
        mAwaitingDescriptionDir.clear();
        _hideDescriptionPrompt();
    }

    void FlightRecorder::_showDescriptionPrompt(){
        //Freeze gameplay so the world does not carry on moving while the player types, and
        //so what they describe stays on screen behind the prompt.
        mPausedMaskBefore = PauseState::getMask();
        PauseState::setMask(mPausedMaskBefore
            | PAUSE_TYPE_PHYSICS | PAUSE_TYPE_NAV_MESH | PAUSE_TYPE_LIFETIME_COMPONENT
            | PAUSE_TYPE_ANIMATIONS | PAUSE_TYPE_TIMERS | PAUSE_TYPE_ENTITY_UPDATE
            | PAUSE_TYPE_PARTICLES | PAUSE_TYPE_SPECIFIC_ANIMATIONS);

        std::shared_ptr<GuiManager> gui = BaseSingleton::getGuiManager();
        if(gui) gui->showCapturePrompt(true);
    }

    void FlightRecorder::_hideDescriptionPrompt(){
        PauseState::setMask(mPausedMaskBefore);

        std::shared_ptr<GuiManager> gui = BaseSingleton::getGuiManager();
        if(gui) gui->showCapturePrompt(false);
    }

    static const char* renderSystemName(SystemSettings::RenderSystemTypes type){
        switch(type){
            case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL: return "OpenGL";
            case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL: return "Metal";
            case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11: return "D3D11";
            case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_VULKAN: return "Vulkan";
            default: return "Unset";
        }
    }

#ifdef DEBUG_SERVER
    static std::string documentToString(const rapidjson::Document& doc){
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        return buffer.GetString();
    }
#endif

    void FlightRecorder::_populateMetadata(CaptureData& data){
        data.engineVersion =
            std::to_string(ENGINE_VERSION_MAJOR) + "." +
            std::to_string(ENGINE_VERSION_MINOR) + "." +
            std::to_string(ENGINE_VERSION_PATCH) + " " + ENGINE_VERSION_SUFFIX;
        data.gitHash = kGitHash;
        data.project = SystemSettings::getProjectName();
        data.renderSystem = renderSystemName(SystemSettings::getCurrentRenderSystem());
        data.commandLine = SystemSettings::getAvSetupFilePath();
        data.timestamp = mCaptureTimestamp;
        data.uptimeSeconds = static_cast<double>(nowNs() - mStartTimeNs) / 1000000000.0;
        data.ringCapacity = mSettings.ringFrames;
        data.everyNthFrame = mSettings.everyNthFrame;
        data.logPath = Log::GetLogFilePath();

        Window* window = BaseSingleton::getWindow();
        if(window){
            data.windowWidth = static_cast<uint32_t>(window->getWidth());
            data.windowHeight = static_cast<uint32_t>(window->getHeight());
        }

#ifdef DEBUG_SERVER
        //The scene and gui inspectors live with the debug server, so these sections are only
        //available when it is compiled in. The manifest records which sections are present.
        {
            rapidjson::Document doc;
            int status = 200;
            SceneInspector::writeSceneTree(doc, status, "", 8, 2000);
            data.sceneJson = documentToString(doc);
        }
        {
            rapidjson::Document doc;
            GuiInspector::writeLabels(doc, false);
            data.guiJson = documentToString(doc);
        }
#endif
    }

    std::string FlightRecorder::_timestamp(){
        const std::time_t now = std::time(nullptr);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &now);
#else
        localtime_r(&now, &tm);
#endif
        char buffer[32];
        //Colons are not legal in a windows path, so the time is separated with dashes.
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H-%M-%S", &tm);
        return buffer;
    }

    bool FlightRecorder::debuggerToolsReceiver(const Event& event){
        if(event.eventId() != EventId::DebuggingToolToggle) return true;

        const DebuggerToolEventToggle& toggle = (DebuggerToolEventToggle&)event;
        if(toggle.t != DebuggerToolToggle::FlightRecorderCapture) return true;

        capture(CaptureReason::Hotkey);
        return true;
    }
}

#endif
