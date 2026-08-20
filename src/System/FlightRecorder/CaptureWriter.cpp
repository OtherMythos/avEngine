#ifdef FLIGHT_RECORDER

#include "CaptureWriter.h"

#include "System/Capture/ImageOps.h"
#include "Logger/Log.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <condition_variable>
#include <deque>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>

namespace AV{

    const char* captureReasonName(CaptureReason reason){
        switch(reason){
            case CaptureReason::Hotkey: return "hotkey";
            case CaptureReason::Script: return "script";
            case CaptureReason::ScriptError: return "scriptError";
            case CaptureReason::DebugServer: return "debugServer";
        }
        return "unknown";
    }

    struct CaptureWriter::Queue{
        std::mutex mutex;
        std::condition_variable cv;
        std::deque<CaptureData> pending;
    };

    CaptureWriter::~CaptureWriter(){
        shutdown();
    }

    void CaptureWriter::initialise(){
        if(mRunning.load()) return;
        mQueue = new Queue();
        mRunning.store(true);
        mThread = std::thread(&CaptureWriter::_threadMain, this);
    }

    void CaptureWriter::submit(CaptureData&& capture){
        if(!mQueue){
            AV_ERROR("Flight recorder capture submitted before the writer started.");
            return;
        }
        mOutstanding.fetch_add(1);
        {
            std::lock_guard<std::mutex> lock(mQueue->mutex);
            mQueue->pending.push_back(std::move(capture));
        }
        mQueue->cv.notify_one();
    }

    bool CaptureWriter::busy() const{
        return mOutstanding.load() > 0;
    }

    void CaptureWriter::shutdown(){
        if(!mRunning.load()){
            delete mQueue;
            mQueue = nullptr;
            return;
        }

        mRunning.store(false);
        mQueue->cv.notify_all();
        if(mThread.joinable()) mThread.join();

        //Anything still queued is written here rather than dropped: a capture the player
        //has already described is the last thing that should be lost to a clean exit.
        for(const CaptureData& capture : mQueue->pending){
            _write(capture);
        }
        mQueue->pending.clear();

        delete mQueue;
        mQueue = nullptr;
        mOutstanding.store(0);
    }

    void CaptureWriter::_threadMain(){
        while(true){
            CaptureData capture;
            {
                std::unique_lock<std::mutex> lock(mQueue->mutex);
                mQueue->cv.wait(lock, [this]{ return !mQueue->pending.empty() || !mRunning.load(); });

                if(mQueue->pending.empty()){
                    if(!mRunning.load()) return;
                    continue;
                }
                capture = std::move(mQueue->pending.front());
                mQueue->pending.pop_front();
            }

            _write(capture);
            mOutstanding.fetch_sub(1);
        }
    }

    static bool writeTextFile(const std::string& path, const std::string& contents){
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if(!file.is_open()) return false;
        file << contents;
        return file.good();
    }

    void CaptureWriter::_write(const CaptureData& capture){
        namespace fs = std::filesystem;

        std::error_code ec;
        const fs::path root(capture.directory);
        fs::create_directories(root / "frames", ec);
        if(ec){
            AV_ERROR("Flight recorder could not create capture directory '{}': {}", capture.directory, ec.message());
            return;
        }

        for(size_t i = 0; i < capture.frames.size(); i++){
            const FrameRecord& record = capture.frames[i];
            if(!record.frame.valid()) continue;

            const std::vector<uint8_t> png = ImageOps::encodePng(record.frame);
            if(png.empty()) continue;

            char name[32];
            snprintf(name, sizeof(name), "frame_%04zu.png", i);
            const fs::path path = root / "frames" / name;
            std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
            if(file.is_open()) file.write(reinterpret_cast<const char*>(png.data()), static_cast<std::streamsize>(png.size()));
        }

        if(capture.fullFrame.valid()){
            const std::vector<uint8_t> png = ImageOps::encodePng(capture.fullFrame);
            if(!png.empty()){
                const fs::path path = root / "frames" / "full.png";
                std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
                if(file.is_open()) file.write(reinterpret_cast<const char*>(png.data()), static_cast<std::streamsize>(png.size()));
            }
        }

        writeTextFile((root / "manifest.json").string(), _buildManifest(capture));
        writeTextFile((root / "timeline.json").string(), _buildTimeline(capture));
        writeTextFile((root / "script.json").string(), _buildScript(capture));

        //Only written when a description was supplied up front; the hotkey flow writes it
        //separately once the player has finished typing.
        if(!capture.description.empty()){
            writeDescription(capture.directory, capture.description);
        }

        if(!capture.sceneJson.empty()) writeTextFile((root / "scene.json").string(), capture.sceneJson);
        if(!capture.guiJson.empty()) writeTextFile((root / "gui.json").string(), capture.guiJson);

        _writeLogTail(capture);

        AV_INFO("Flight recorder wrote capture to {}", capture.directory);
    }

    bool CaptureWriter::writeDescription(const std::string& directory, const std::string& description){
        std::error_code ec;
        std::filesystem::create_directories(directory, ec);
        return writeTextFile((std::filesystem::path(directory) / "description.txt").string(), description);
    }

    static std::string documentToString(const rapidjson::Document& doc){
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        return buffer.GetString();
    }

    std::string CaptureWriter::_buildManifest(const CaptureData& capture){
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        doc.AddMember("engineVersion", rapidjson::Value(capture.engineVersion.c_str(), allocator), allocator);
        doc.AddMember("gitHash", rapidjson::Value(capture.gitHash.c_str(), allocator), allocator);
        doc.AddMember("project", rapidjson::Value(capture.project.c_str(), allocator), allocator);
        doc.AddMember("renderSystem", rapidjson::Value(capture.renderSystem.c_str(), allocator), allocator);
        doc.AddMember("commandLine", rapidjson::Value(capture.commandLine.c_str(), allocator), allocator);
        doc.AddMember("timestamp", rapidjson::Value(capture.timestamp.c_str(), allocator), allocator);
        doc.AddMember("reason", rapidjson::Value(captureReasonName(capture.reason), allocator), allocator);
        doc.AddMember("uptimeSeconds", capture.uptimeSeconds, allocator);

        rapidjson::Value window(rapidjson::kObjectType);
        window.AddMember("width", capture.windowWidth, allocator);
        window.AddMember("height", capture.windowHeight, allocator);
        doc.AddMember("window", window, allocator);

        rapidjson::Value ring(rapidjson::kObjectType);
        ring.AddMember("frames", static_cast<uint64_t>(capture.frames.size()), allocator);
        ring.AddMember("capacity", capture.ringCapacity, allocator);
        ring.AddMember("everyNthFrame", capture.everyNthFrame, allocator);
        if(!capture.frames.empty()){
            ring.AddMember("captureWidth", capture.frames.back().frame.width, allocator);
            ring.AddMember("captureHeight", capture.frames.back().frame.height, allocator);
        }
        doc.AddMember("ring", ring, allocator);

        rapidjson::Value sections(rapidjson::kObjectType);
        sections.AddMember("frames", !capture.frames.empty(), allocator);
        sections.AddMember("fullFrame", capture.fullFrame.valid(), allocator);
        sections.AddMember("script", !capture.events.empty(), allocator);
        sections.AddMember("backtrace", !capture.backtrace.empty(), allocator);
        sections.AddMember("scene", !capture.sceneJson.empty(), allocator);
        sections.AddMember("gui", !capture.guiJson.empty(), allocator);
        doc.AddMember("sections", sections, allocator);

        //A hotkey capture is taken while the vm is idle, so it has no live stack. Saying so
        //here stops a triaging agent reading the empty backtrace as "no script was running".
        const char* backtraceNote = capture.backtrace.empty()
            ? "No live squirrel stack: the capture was taken while the vm was idle. Use the per frame deepestStack in timeline.json and the call trace in script.json instead."
            : "Live squirrel stack captured while script code was executing.";
        doc.AddMember("backtraceNote", rapidjson::Value(backtraceNote, allocator), allocator);

        return documentToString(doc);
    }

    std::string CaptureWriter::_buildTimeline(const CaptureData& capture){
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value frames(rapidjson::kArrayType);
        for(size_t i = 0; i < capture.frames.size(); i++){
            const FrameRecord& record = capture.frames[i];

            rapidjson::Value entry(rapidjson::kObjectType);
            entry.AddMember("index", static_cast<uint64_t>(i), allocator);
            entry.AddMember("frameNumber", record.frameNumber, allocator);
            entry.AddMember("timeMs", record.timeMs, allocator);
            entry.AddMember("frameTimeMs", record.frameTimeMs, allocator);
            entry.AddMember("fps", record.fps, allocator);
            entry.AddMember("dhash", rapidjson::Value(ImageOps::hashToHex(record.dHash).c_str(), allocator), allocator);

            char imageName[32];
            snprintf(imageName, sizeof(imageName), "frames/frame_%04zu.png", i);
            entry.AddMember("image", rapidjson::Value(imageName, allocator), allocator);

            rapidjson::Value script(rapidjson::kObjectType);
            script.AddMember("eventBegin", record.scriptEventBegin, allocator);
            script.AddMember("eventEnd", record.scriptEventEnd, allocator);
            script.AddMember("eventCount", record.scriptEventEnd - record.scriptEventBegin, allocator);
            rapidjson::Value stack(rapidjson::kArrayType);
            for(uint32_t funcIndex : record.deepestStack){
                const char* name = funcIndex < capture.functions.size() ? capture.functions[funcIndex].name.c_str() : "unknown";
                stack.PushBack(rapidjson::Value(name, allocator), allocator);
            }
            script.AddMember("deepestStack", stack, allocator);
            entry.AddMember("script", script, allocator);

            rapidjson::Value marks(rapidjson::kArrayType);
            for(const std::string& mark : record.marks){
                marks.PushBack(rapidjson::Value(mark.c_str(), allocator), allocator);
            }
            entry.AddMember("marks", marks, allocator);

            rapidjson::Value watches(rapidjson::kObjectType);
            for(const WatchValue& watch : record.watches){
                watches.AddMember(rapidjson::Value(watch.name.c_str(), allocator),
                                  rapidjson::Value(watch.value.c_str(), allocator), allocator);
            }
            entry.AddMember("watches", watches, allocator);

            frames.PushBack(entry, allocator);
        }
        doc.AddMember("frames", frames, allocator);

        return documentToString(doc);
    }

    std::string CaptureWriter::_buildScript(const CaptureData& capture){
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value functions(rapidjson::kArrayType);
        for(size_t i = 0; i < capture.functions.size(); i++){
            const TracedFunction& func = capture.functions[i];
            rapidjson::Value entry(rapidjson::kObjectType);
            entry.AddMember("index", static_cast<uint64_t>(i), allocator);
            entry.AddMember("name", rapidjson::Value(func.name.c_str(), allocator), allocator);
            entry.AddMember("source", rapidjson::Value(func.source.c_str(), allocator), allocator);
            entry.AddMember("declLine", func.declLine, allocator);
            functions.PushBack(entry, allocator);
        }
        doc.AddMember("functions", functions, allocator);

        doc.AddMember("eventsBegin", capture.eventsBegin, allocator);
        rapidjson::Value events(rapidjson::kArrayType);
        for(const ScriptEvent& event : capture.events){
            rapidjson::Value entry(rapidjson::kObjectType);
            entry.AddMember("func", event.funcIndex, allocator);
            entry.AddMember("type", rapidjson::Value(event.type == ScriptEventType::Call ? "call" : "return", allocator), allocator);
            entry.AddMember("line", event.line, allocator);
            entry.AddMember("timeNs", event.timeNs, allocator);
            events.PushBack(entry, allocator);
        }
        doc.AddMember("events", events, allocator);

        rapidjson::Value backtrace(rapidjson::kArrayType);
        for(const BacktraceFrame& frame : capture.backtrace){
            rapidjson::Value entry(rapidjson::kObjectType);
            entry.AddMember("function", rapidjson::Value(frame.function.c_str(), allocator), allocator);
            entry.AddMember("source", rapidjson::Value(frame.source.c_str(), allocator), allocator);
            entry.AddMember("line", frame.line, allocator);

            rapidjson::Value locals(rapidjson::kObjectType);
            for(const std::pair<std::string, std::string>& local : frame.locals){
                locals.AddMember(rapidjson::Value(local.first.c_str(), allocator),
                                 rapidjson::Value(local.second.c_str(), allocator), allocator);
            }
            entry.AddMember("locals", locals, allocator);
            backtrace.PushBack(entry, allocator);
        }
        doc.AddMember("backtrace", backtrace, allocator);

        return documentToString(doc);
    }

    void CaptureWriter::_writeLogTail(const CaptureData& capture){
        if(capture.logPath.empty()) return;

        std::ifstream log(capture.logPath);
        if(!log.is_open()) return;

        static const size_t MAX_LINES = 500;
        std::deque<std::string> lines;
        std::string line;
        while(std::getline(log, line)){
            lines.push_back(line);
            if(lines.size() > MAX_LINES) lines.pop_front();
        }

        std::ostringstream out;
        for(const std::string& l : lines) out << l << "\n";
        writeTextFile((std::filesystem::path(capture.directory) / "log_tail.txt").string(), out.str());
    }
}

#endif
