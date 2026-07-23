#ifdef SCRIPT_PROFILER

#include "ScriptProfiler.h"

#include "Scripting/SquirrelHookDispatcher.h"
#include "Logger/Log.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <algorithm>
#include <chrono>
#include <fstream>

namespace AV{
    static const char* UNKNOWN_SOURCE = "<unknown>";
    static const char* ANONYMOUS_FUNCTION = "<anonymous>";

    //Out of line definitions, so these can be bound to a const reference (std::min).
    const uint32_t ScriptProfiler::ROOT_INDEX;
    const size_t ScriptProfiler::FRAME_HISTORY;

    HSQUIRRELVM ScriptProfiler::mVm = 0;
    ScriptProfilerSettings ScriptProfiler::mSettings;
    bool ScriptProfiler::mEnabled = false;
    bool ScriptProfiler::mRunning = false;

    std::vector<ScriptProfiler::FunctionRecord> ScriptProfiler::mFunctions;
    std::unordered_map<ScriptProfiler::FuncKey, uint32_t, ScriptProfiler::FuncKeyHash> ScriptProfiler::mIndexByPointer;
    std::unordered_map<std::string, uint32_t> ScriptProfiler::mIndexByName;

    std::vector<ScriptProfiler::StackFrame> ScriptProfiler::mStack;
    std::unordered_map<uint64_t, ScriptProfiler::EdgeRecord> ScriptProfiler::mEdges;
    std::unordered_map<uint64_t, ScriptProfiler::LineRecord> ScriptProfiler::mLines;

    std::vector<ScriptProfiler::FrameEntry> ScriptProfiler::mFrameHistory;
    uint64_t ScriptProfiler::mFrameHistoryCount = 0;

    uint64_t ScriptProfiler::mStartNs = 0;
    uint64_t ScriptProfiler::mStoppedNs = 0;
    uint64_t ScriptProfiler::mTotalScriptNs = 0;
    uint64_t ScriptProfiler::mTotalCalls = 0;
    uint64_t ScriptProfiler::mFramesElapsed = 0;
    uint64_t ScriptProfiler::mLineExecutions = 0;
    ScriptProfiler::LineRecord* ScriptProfiler::mOpenLine = 0;
    uint64_t ScriptProfiler::mOpenLineNs = 0;

    uint64_t ScriptProfiler::mFrameScriptNs = 0;
    uint32_t ScriptProfiler::mFrameCalls = 0;
    uint32_t ScriptProfiler::mFrameTopRootFunc = ScriptProfiler::ROOT_INDEX;
    uint64_t ScriptProfiler::mFrameTopRootNs = 0;

    static uint64_t steadyClockNs(){
        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    ScriptProfiler::ClockFunction ScriptProfiler::mClock = steadyClockNs;

    void ScriptProfiler::initialise(HSQUIRRELVM vm, const ScriptProfilerSettings& settings){
        mVm = vm;
        mSettings = settings;

        mFunctions.clear();
        mIndexByPointer.clear();
        mIndexByName.clear();
        mStack.clear();
        mStack.reserve(64);
        mFrameHistory.assign(settings.collectFrames ? FRAME_HISTORY : 0, FrameEntry());
        mFrameHistoryCount = 0;

        _clearCounters();

        mEnabled = true;
        mRunning = true;
        mStartNs = mClock();

        SquirrelHookDispatcher::setConsumer(SquirrelHookDispatcher::Consumer::PROFILER, _hook);

        AV_INFO("Squirrel profiler collecting. Per line timing {}.",
            settings.collectLines ? "enabled" : "disabled");
    }

    void ScriptProfiler::shutdown(){
        if(!mEnabled) return;

        //Stop before reporting so nothing mutates the data while it is read.
        stop();
        SquirrelHookDispatcher::setConsumer(SquirrelHookDispatcher::Consumer::PROFILER, 0);

        {
            //A short summary in the log is what the flag gives you on its own.
            ProfileQuery query;
            query.maxFunctions = 10;
            ProfileReport report;
            buildReport(report, query);
            AV_INFO("\n{}", ProfileReportWriter::toText(report));
        }

        if(!mSettings.outputPath.empty()){
            const std::string error = writeReportToFile(mSettings.outputPath, pathWantsJson(mSettings.outputPath));
            if(error.empty()){
                AV_INFO("Squirrel profile written to {}", mSettings.outputPath);
            }else{
                AV_ERROR("Failed to write the squirrel profile to {}: {}", mSettings.outputPath, error);
            }
        }

        _releasePinnedStrings();

        mFunctions.clear();
        mIndexByPointer.clear();
        mIndexByName.clear();
        mStack.clear();
        mEdges.clear();
        mLines.clear();
        mOpenLine = 0;
        mFrameHistory.clear();

        mEnabled = false;
        mVm = 0;
    }

    void ScriptProfiler::start(){
        if(!mEnabled || mRunning) return;

        //Exclude the time spent stopped from the elapsed window, so the percentages
        //stay meaningful across a stop/start.
        mStartNs += mClock() - mStoppedNs;
        mRunning = true;
    }

    void ScriptProfiler::stop(){
        if(!mEnabled || !mRunning) return;

        mRunning = false;
        mStoppedNs = mClock();
        //Open calls will never see their 'r' event now, and their start times would be
        //meaningless if collection resumes later.
        mStack.clear();
        mOpenLine = 0;
        for(FunctionRecord& record : mFunctions){
            record.activeCount = 0;
        }
    }

    void ScriptProfiler::reset(){
        if(!mEnabled) return;

        _clearCounters();

        //Keep the discovered functions so ids stay stable across a reset, but drop
        //everything measured about them.
        for(FunctionRecord& record : mFunctions){
            record.callCount = 0;
            record.inclusiveNs = 0;
            record.exclusiveNs = 0;
            record.minInclusiveNs = 0;
            record.maxInclusiveNs = 0;
            record.maxStackDepth = 0;
            record.activeCount = 0;
        }

        mStack.clear();
        mStartNs = mClock();
        mStoppedNs = mStartNs;
    }

    void ScriptProfiler::_clearCounters(){
        mEdges.clear();
        mLines.clear();
        mOpenLine = 0;
        mFrameHistoryCount = 0;
        mTotalScriptNs = 0;
        mTotalCalls = 0;
        mFramesElapsed = 0;
        mLineExecutions = 0;
        mFrameScriptNs = 0;
        mFrameCalls = 0;
        mFrameTopRootFunc = ROOT_INDEX;
        mFrameTopRootNs = 0;
    }

    void ScriptProfiler::notifyFrameBoundary(){
        if(!mEnabled) return;

        mFramesElapsed++;

        if(mRunning && mSettings.collectFrames && !mFrameHistory.empty()){
            FrameEntry& entry = mFrameHistory[mFrameHistoryCount % FRAME_HISTORY];
            entry.frameIndex = mFramesElapsed;
            entry.scriptNs = mFrameScriptNs;
            entry.calls = mFrameCalls;
            entry.topRootFunc = mFrameTopRootFunc;
            entry.topRootNs = mFrameTopRootNs;
            mFrameHistoryCount++;
        }

        mFrameScriptNs = 0;
        mFrameCalls = 0;
        mFrameTopRootFunc = ROOT_INDEX;
        mFrameTopRootNs = 0;
    }

    void ScriptProfiler::_hook(HSQUIRRELVM vm, SQInteger type, const SQChar* sourceName, SQInteger line, const SQChar* funcName){
        if(!mRunning) return;

        switch(type){
            case _SC('c'): _onCall(vm, sourceName, line, funcName); break;
            case _SC('r'): _onReturn(); break;
            case _SC('l'):
                //Every executed line arrives here, so the check is worth having.
                if(mSettings.collectLines) _onLine(line);
                break;
            default: break;
        }
    }

    void ScriptProfiler::_onCall(HSQUIRRELVM vm, const SQChar* source, SQInteger line, const SQChar* funcName){
        //Whatever the caller was doing stops here; the callee's time is its own.
        _closeOpenLine();

        const FuncKey key{ source, funcName, line };
        const uint32_t index = _resolveFunction(vm, key);

        FunctionRecord& record = mFunctions[index];
        record.callCount++;
        record.activeCount++;

        mStack.push_back({ index, mClock(), 0 });

        if(mStack.size() > record.maxStackDepth){
            record.maxStackDepth = static_cast<uint32_t>(mStack.size());
        }

        mFrameCalls++;
        mTotalCalls++;
    }

    void ScriptProfiler::_onReturn(){
        _closeOpenLine();

        //Empty when collection began part way through a call. The matching 'c' was
        //never seen, so there is nothing to close.
        if(mStack.empty()) return;

        const StackFrame frame = mStack.back();
        mStack.pop_back();

        const uint64_t now = mClock();
        const uint64_t inclusive = now > frame.startNs ? now - frame.startNs : 0;
        const uint64_t exclusive = inclusive > frame.childNs ? inclusive - frame.childNs : 0;

        FunctionRecord& record = mFunctions[frame.funcIndex];
        record.exclusiveNs += exclusive;
        if(record.activeCount > 0) record.activeCount--;

        //Only the outermost frame of a recursive chain contributes inclusive time.
        //Counting every level would add the same nanoseconds once per level of depth.
        if(record.activeCount == 0){
            record.inclusiveNs += inclusive;
            if(inclusive > record.maxInclusiveNs) record.maxInclusiveNs = inclusive;
            if(record.minInclusiveNs == 0 || inclusive < record.minInclusiveNs) record.minInclusiveNs = inclusive;
        }

        const uint32_t caller = mStack.empty() ? ROOT_INDEX : mStack.back().funcIndex;
        if(!mStack.empty()){
            mStack.back().childNs += inclusive;
        }else{
            //A root call finished, so all of it is script time attributable to this frame.
            mFrameScriptNs += inclusive;
            mTotalScriptNs += inclusive;
            if(inclusive > mFrameTopRootNs){
                mFrameTopRootNs = inclusive;
                mFrameTopRootFunc = frame.funcIndex;
            }
        }

        if(mSettings.collectEdges){
            EdgeRecord& edge = mEdges[(static_cast<uint64_t>(caller) << 32) | frame.funcIndex];
            edge.callCount++;
            edge.inclusiveNs += inclusive;
        }
    }

    void ScriptProfiler::_closeOpenLine(){
        if(!mOpenLine) return;

        const uint64_t now = mClock();
        if(now > mOpenLineNs) mOpenLine->selfNs += now - mOpenLineNs;
        mOpenLine = 0;
    }

    void ScriptProfiler::_onLine(SQInteger line){
        //Squirrel emits this before executing the line, so the previous line's time is
        //everything up to now. Timing each line directly rather than sampling matters
        //here: script tends to run in short bursts once per frame, and any wall clock
        //sampler would land on the first line of the burst nearly every time.
        const uint64_t now = mClock();
        if(mOpenLine && now > mOpenLineNs){
            mOpenLine->selfNs += now - mOpenLineNs;
        }

        //No enclosing call means collection started mid execution; nothing to attribute.
        if(mStack.empty()){
            mOpenLine = 0;
            return;
        }

        const uint32_t leaf = mStack.back().funcIndex;
        LineRecord& record = mLines[(static_cast<uint64_t>(leaf) << 32) | static_cast<uint32_t>(line)];
        record.executions++;
        mLineExecutions++;

        //Addresses of unordered_map elements are stable, so this stays valid until the
        //map is cleared.
        mOpenLine = &record;
        mOpenLineNs = now;
    }

    uint32_t ScriptProfiler::_resolveFunction(HSQUIRRELVM vm, const FuncKey& key){
        auto it = mIndexByPointer.find(key);
        if(it != mIndexByPointer.end()) return it->second;

        return _createFunction(vm, key);
    }

    uint32_t ScriptProfiler::_createFunction(HSQUIRRELVM vm, const FuncKey& key){
        const std::string source = key.source ? key.source : UNKNOWN_SOURCE;
        //Squirrel reports no name for an anonymous function.
        const std::string name = key.name ? key.name : ANONYMOUS_FUNCTION;

        const std::string mergeKey = source + "\n" + name + "\n" + std::to_string(static_cast<long long>(key.declLine));
        auto it = mIndexByName.find(mergeKey);
        if(it != mIndexByName.end()){
            //Same function, fresh interned strings after a reload. Cache the new
            //pointers against the record it already has.
            mIndexByPointer[key] = it->second;
            return it->second;
        }

        const uint32_t index = static_cast<uint32_t>(mFunctions.size());
        mFunctions.emplace_back();
        FunctionRecord& record = mFunctions.back();
        record.source = source;
        record.name = name;
        record.declLine = static_cast<int>(key.declLine);
        sq_resetobject(&record.sourceObj);
        sq_resetobject(&record.nameObj);

        if(vm){
            if(key.source) _pinString(vm, key.source, record.sourceObj);
            if(key.name) _pinString(vm, key.name, record.nameObj);
            record.pinned = true;
        }

        mIndexByName[mergeKey] = index;
        mIndexByPointer[key] = index;
        return index;
    }

    void ScriptProfiler::_pinString(HSQUIRRELVM vm, const SQChar* str, HSQOBJECT& out){
        //Safe inside the hook: squirrel disables it for the duration of the callback,
        //and the stack is restored before returning.
        const SQInteger top = sq_gettop(vm);
        sq_pushstring(vm, str, -1);
        sq_getstackobj(vm, -1, &out);
        sq_addref(vm, &out);
        sq_settop(vm, top);
    }

    void ScriptProfiler::_releasePinnedStrings(){
        if(!mVm) return;

        for(FunctionRecord& record : mFunctions){
            if(!record.pinned) continue;
            if(!sq_isnull(record.sourceObj)) sq_release(mVm, &record.sourceObj);
            if(!sq_isnull(record.nameObj)) sq_release(mVm, &record.nameObj);
            record.pinned = false;
        }
    }

    double ScriptProfiler::_elapsedMs(){
        if(!mEnabled) return 0.0;
        const uint64_t end = mRunning ? mClock() : mStoppedNs;
        return end > mStartNs ? static_cast<double>(end - mStartNs) / 1000000.0 : 0.0;
    }

    void ScriptProfiler::buildReport(ProfileReport& out, const ProfileQuery& query){
        out.totals.enabled = mEnabled;
        out.totals.running = mRunning;
        if(!mEnabled) return;

        out.totals.elapsedMs = _elapsedMs();
        out.totals.scriptMs = static_cast<double>(mTotalScriptNs) / 1000000.0;
        out.totals.totalCalls = mTotalCalls;
        out.totals.framesElapsed = mFramesElapsed;
        out.totals.lineExecutions = mLineExecutions;
        out.totals.functionsSeen = static_cast<uint32_t>(mFunctions.size());

        //Percentages are against measured squirrel time, not wall time, so they add to
        //100 across the table however much of the frame squirrel actually accounts for.
        const double totalExclusiveMs = []() -> double {
            uint64_t total = 0;
            for(const FunctionRecord& record : mFunctions) total += record.exclusiveNs;
            return static_cast<double>(total) / 1000000.0;
        }();

        for(uint32_t i = 0; i < mFunctions.size(); i++){
            const FunctionRecord& record = mFunctions[i];
            if(record.callCount < query.minCalls) continue;
            if(query.singleFunction && i != query.functionId) continue;
            //A function discovered before a reset but not called since carries no
            //information, and would only pad the table.
            if(record.callCount == 0) continue;

            ProfileReport::Function entry;
            entry.id = i;
            entry.source = record.source;
            entry.name = record.name;
            entry.declLine = record.declLine;
            entry.callCount = record.callCount;
            entry.inclusiveMs = static_cast<double>(record.inclusiveNs) / 1000000.0;
            entry.exclusiveMs = static_cast<double>(record.exclusiveNs) / 1000000.0;
            entry.avgUs = record.callCount > 0
                ? (static_cast<double>(record.inclusiveNs) / 1000.0) / static_cast<double>(record.callCount)
                : 0.0;
            entry.minUs = static_cast<double>(record.minInclusiveNs) / 1000.0;
            entry.maxUs = static_cast<double>(record.maxInclusiveNs) / 1000.0;
            entry.pctExclusive = totalExclusiveMs > 0.0 ? (entry.exclusiveMs / totalExclusiveMs) * 100.0 : 0.0;
            entry.callsPerFrame = mFramesElapsed > 0
                ? static_cast<double>(record.callCount) / static_cast<double>(mFramesElapsed)
                : 0.0;
            entry.maxStackDepth = record.maxStackDepth;

            out.functions.push_back(entry);
        }

        const ProfileQuery::Sort sort = query.sort;
        std::sort(out.functions.begin(), out.functions.end(),
            [sort](const ProfileReport::Function& a, const ProfileReport::Function& b){
                switch(sort){
                    case ProfileQuery::Sort::INCLUSIVE: return a.inclusiveMs > b.inclusiveMs;
                    case ProfileQuery::Sort::CALLS: return a.callCount > b.callCount;
                    case ProfileQuery::Sort::AVERAGE: return a.avgUs > b.avgUs;
                    case ProfileQuery::Sort::EXCLUSIVE:
                    default: return a.exclusiveMs > b.exclusiveMs;
                }
            });

        if(query.maxFunctions > 0 && out.functions.size() > query.maxFunctions){
            out.totals.functionsOmitted = static_cast<uint32_t>(out.functions.size() - query.maxFunctions);
            out.functions.resize(query.maxFunctions);
        }

        if(query.includeEdges){
            for(const auto& pair : mEdges){
                const uint32_t caller = static_cast<uint32_t>(pair.first >> 32);
                const uint32_t callee = static_cast<uint32_t>(pair.first & 0xFFFFFFFF);
                //A single function view wants only the edges touching it.
                if(query.singleFunction && caller != query.functionId && callee != query.functionId) continue;

                ProfileReport::Edge edge;
                edge.callerId = caller;
                edge.calleeId = callee;
                edge.callCount = pair.second.callCount;
                edge.inclusiveMs = static_cast<double>(pair.second.inclusiveNs) / 1000000.0;
                out.edges.push_back(edge);
            }
            std::sort(out.edges.begin(), out.edges.end(),
                [](const ProfileReport::Edge& a, const ProfileReport::Edge& b){
                    return a.inclusiveMs > b.inclusiveMs;
                });
            if(query.maxEdges > 0 && out.edges.size() > query.maxEdges) out.edges.resize(query.maxEdges);
        }

        if(query.includeLines){
            uint64_t totalLineNs = 0;
            for(const auto& pair : mLines) totalLineNs += pair.second.selfNs;

            for(const auto& pair : mLines){
                const uint32_t func = static_cast<uint32_t>(pair.first >> 32);
                if(query.singleFunction && func != query.functionId) continue;

                ProfileReport::Line line;
                line.functionId = func;
                if(func < mFunctions.size()){
                    line.functionName = mFunctions[func].name;
                    line.source = mFunctions[func].source;
                }
                line.line = static_cast<int>(pair.first & 0xFFFFFFFF);
                line.executions = pair.second.executions;
                line.selfMs = static_cast<double>(pair.second.selfNs) / 1000000.0;
                line.pctSelf = totalLineNs > 0
                    ? (static_cast<double>(pair.second.selfNs) / static_cast<double>(totalLineNs)) * 100.0
                    : 0.0;
                out.hotLines.push_back(line);
            }
            std::sort(out.hotLines.begin(), out.hotLines.end(),
                [](const ProfileReport::Line& a, const ProfileReport::Line& b){
                    if(a.selfMs != b.selfMs) return a.selfMs > b.selfMs;
                    return a.executions > b.executions;
                });
            if(query.maxLines > 0 && out.hotLines.size() > query.maxLines) out.hotLines.resize(query.maxLines);
        }

        if(query.includeFrames && !mFrameHistory.empty()){
            const uint64_t available = std::min<uint64_t>(mFrameHistoryCount, FRAME_HISTORY);
            const uint64_t first = mFrameHistoryCount - available;
            for(uint64_t i = 0; i < available; i++){
                const FrameEntry& source = mFrameHistory[(first + i) % FRAME_HISTORY];
                ProfileReport::Frame frame;
                frame.frameIndex = source.frameIndex;
                frame.scriptMs = static_cast<double>(source.scriptNs) / 1000000.0;
                frame.calls = source.calls;
                frame.topRootFunctionId = source.topRootFunc;
                frame.topRootMs = static_cast<double>(source.topRootNs) / 1000000.0;
                out.frames.push_back(frame);
            }
            if(query.worstFramesFirst){
                std::sort(out.frames.begin(), out.frames.end(),
                    [](const ProfileReport::Frame& a, const ProfileReport::Frame& b){
                        return a.scriptMs > b.scriptMs;
                    });
            }
            if(query.maxFrames > 0 && out.frames.size() > query.maxFrames){
                //Chronological output keeps the most recent frames, which are the ones
                //a caller watching live cares about.
                if(query.worstFramesFirst){
                    out.frames.resize(query.maxFrames);
                }else{
                    out.frames.erase(out.frames.begin(), out.frames.end() - query.maxFrames);
                }
            }
        }
    }

    bool ScriptProfiler::pathWantsJson(const std::string& path){
        if(path.size() < 5) return false;
        const std::string extension = path.substr(path.size() - 5);
        return extension == ".json" || extension == ".JSON";
    }

    std::string ScriptProfiler::writeReportToFile(const std::string& path, bool json){
        if(!mEnabled) return "the profiler is not enabled";

        //A file has no context budget, so it gets everything.
        ProfileQuery query;
        query.maxFunctions = 0;
        query.includeEdges = true;
        query.includeLines = true;
        query.includeFrames = true;
        query.maxEdges = 0;
        query.maxLines = 0;
        query.maxFrames = 0;

        ProfileReport report;
        buildReport(report, query);

        std::string contents;
        if(json){
            rapidjson::Document doc;
            ProfileReportWriter::toJson(report, doc);
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            contents = buffer.GetString();
        }else{
            contents = ProfileReportWriter::toText(report);
        }

        std::ofstream file(path.c_str(), std::ios::out | std::ios::trunc);
        if(!file.is_open()) return "could not open the file for writing";
        file << contents;
        if(!file.good()) return "the write failed";
        file.close();

        return "";
    }

    void ScriptProfiler::_setClockForTesting(ClockFunction func){
        mClock = func ? func : steadyClockNs;
    }

    void ScriptProfiler::_hookForTesting(SQInteger type, const SQChar* source, SQInteger line, const SQChar* funcName){
        _hook(0, type, source, line, funcName);
    }
}

#endif
