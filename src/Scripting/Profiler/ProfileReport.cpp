#ifdef SCRIPT_PROFILER

#include "ProfileReport.h"

#include <cstdarg>
#include <cstdio>

namespace AV{
    const uint32_t ProfileReport::ROOT_ID;

    const ProfileReport::Function* ProfileReport::findFunction(uint32_t id) const{
        for(const Function& f : functions){
            if(f.id == id) return &f;
        }
        return 0;
    }

    bool ProfileQuery::parseSort(const std::string& str, Sort& out){
        if(str == "exclusive"){ out = Sort::EXCLUSIVE; return true; }
        if(str == "inclusive"){ out = Sort::INCLUSIVE; return true; }
        if(str == "calls"){ out = Sort::CALLS; return true; }
        if(str == "avg" || str == "average"){ out = Sort::AVERAGE; return true; }
        return false;
    }

    const char* ProfileQuery::sortName(Sort sort){
        switch(sort){
            case Sort::INCLUSIVE: return "inclusive";
            case Sort::CALLS: return "calls";
            case Sort::AVERAGE: return "avg";
            case Sort::EXCLUSIVE:
            default: return "exclusive";
        }
    }

    namespace{
        //Paths in a report are absolute and long; the file name is what identifies the
        //script to a reader.
        std::string baseName(const std::string& path){
            const size_t index = path.find_last_of("/\\");
            if(index == std::string::npos) return path;
            return path.substr(index + 1);
        }

        std::string location(const ProfileReport::Function& func){
            return baseName(func.source) + ":" + std::to_string(func.declLine);
        }

        //Keeps a column from being blown out by a long name.
        std::string truncate(const std::string& str, size_t maxLength){
            if(str.size() <= maxLength) return str;
            if(maxLength <= 3) return str.substr(0, maxLength);
            return str.substr(0, maxLength - 3) + "...";
        }

        std::string functionLabel(const ProfileReport& report, uint32_t id){
            if(id == ProfileReport::ROOT_ID) return "<engine>";
            const ProfileReport::Function* func = report.findFunction(id);
            if(!func) return "<function " + std::to_string(id) + ">";
            return func->name + " (" + location(*func) + ")";
        }

        void appendLine(std::string& out, const char* format, ...){
            char buffer[512];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            out += buffer;
            out += "\n";
        }
    }

    std::string ProfileReportWriter::toText(const ProfileReport& report){
        const ProfileReport::Totals& totals = report.totals;

        std::string out;
        out += "==== Squirrel script profile ====\n";

        if(!totals.enabled){
            out += "The profiler is not enabled. Start the engine with --profileScripts.\n";
            return out;
        }

        const double elapsedSeconds = totals.elapsedMs / 1000.0;
        const double scriptSeconds = totals.scriptMs / 1000.0;
        const double scriptPct = totals.elapsedMs > 0.0 ? (totals.scriptMs / totals.elapsedMs) * 100.0 : 0.0;

        appendLine(out, "%s for %.2fs over %llu frames", totals.running ? "Collecting" : "Stopped, collected",
            elapsedSeconds, (unsigned long long)totals.framesElapsed);
        appendLine(out, "Script time %.3fs (%.1f%% of wall) | %llu calls | %u functions | %llu lines executed",
            scriptSeconds, scriptPct, (unsigned long long)totals.totalCalls,
            totals.functionsSeen, (unsigned long long)totals.lineExecutions);

        if(report.functions.empty()){
            out += "\nNo squirrel functions were executed while profiling.\n";
            return out;
        }

        out += "\n";
        appendLine(out, "%10s %10s %12s %10s %10s %6s  %s",
            "excl ms", "incl ms", "calls", "avg us", "max us", "excl%", "function");
        out += std::string(84, '-') + "\n";

        for(const ProfileReport::Function& func : report.functions){
            appendLine(out, "%10.2f %10.2f %12llu %10.2f %10.2f %5.1f%%  %s  %s",
                func.exclusiveMs, func.inclusiveMs, (unsigned long long)func.callCount,
                func.avgUs, func.maxUs, func.pctExclusive,
                truncate(func.name, 34).c_str(), location(func).c_str());
        }

        if(totals.functionsOmitted > 0){
            appendLine(out, "... %u more functions not shown.", totals.functionsOmitted);
        }

        if(!report.edges.empty()){
            out += "\nCall edges (caller -> callee):\n";
            for(const ProfileReport::Edge& edge : report.edges){
                appendLine(out, "%12llu calls %10.2f ms  %s -> %s",
                    (unsigned long long)edge.callCount, edge.inclusiveMs,
                    truncate(functionLabel(report, edge.callerId), 44).c_str(),
                    truncate(functionLabel(report, edge.calleeId), 44).c_str());
            }
        }

        if(!report.hotLines.empty()){
            out += "\nHottest lines (self time, excludes squirrel functions the line called):\n";
            for(const ProfileReport::Line& line : report.hotLines){
                appendLine(out, "%10.2f ms %5.1f%% %12llu runs  %s:%d  in %s",
                    line.selfMs, line.pctSelf, (unsigned long long)line.executions,
                    baseName(line.source).c_str(), line.line,
                    truncate(line.functionName, 30).c_str());
            }
        }

        if(!report.frames.empty()){
            out += "\nFrames:\n";
            for(const ProfileReport::Frame& frame : report.frames){
                appendLine(out, "frame %10llu  %8.3f ms script  %6u calls  worst root %.3f ms in %s",
                    (unsigned long long)frame.frameIndex, frame.scriptMs, frame.calls,
                    frame.topRootMs, truncate(functionLabel(report, frame.topRootFunctionId), 44).c_str());
            }
        }

        return out;
    }

    void ProfileReportWriter::toJson(const ProfileReport& report, rapidjson::Document& doc){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        const ProfileReport::Totals& totals = report.totals;
        doc.AddMember("enabled", totals.enabled, allocator);
        doc.AddMember("running", totals.running, allocator);
        doc.AddMember("elapsedMs", totals.elapsedMs, allocator);
        doc.AddMember("scriptMs", totals.scriptMs, allocator);
        doc.AddMember("totalCalls", totals.totalCalls, allocator);
        doc.AddMember("framesElapsed", totals.framesElapsed, allocator);
        doc.AddMember("lineExecutions", totals.lineExecutions, allocator);
        doc.AddMember("functionsSeen", totals.functionsSeen, allocator);
        doc.AddMember("functionsOmitted", totals.functionsOmitted, allocator);

        rapidjson::Value functions(rapidjson::kArrayType);
        for(const ProfileReport::Function& func : report.functions){
            rapidjson::Value entry(rapidjson::kObjectType);
            entry.AddMember("id", func.id, allocator);
            entry.AddMember("name", rapidjson::Value(func.name.c_str(), allocator), allocator);
            entry.AddMember("source", rapidjson::Value(func.source.c_str(), allocator), allocator);
            entry.AddMember("line", func.declLine, allocator);
            entry.AddMember("calls", func.callCount, allocator);
            entry.AddMember("inclusiveMs", func.inclusiveMs, allocator);
            entry.AddMember("exclusiveMs", func.exclusiveMs, allocator);
            entry.AddMember("avgUs", func.avgUs, allocator);
            entry.AddMember("minUs", func.minUs, allocator);
            entry.AddMember("maxUs", func.maxUs, allocator);
            entry.AddMember("pctExclusive", func.pctExclusive, allocator);
            entry.AddMember("callsPerFrame", func.callsPerFrame, allocator);
            entry.AddMember("maxStackDepth", func.maxStackDepth, allocator);
            functions.PushBack(entry, allocator);
        }
        doc.AddMember("functions", functions, allocator);

        if(!report.edges.empty()){
            rapidjson::Value edges(rapidjson::kArrayType);
            for(const ProfileReport::Edge& edge : report.edges){
                rapidjson::Value entry(rapidjson::kObjectType);
                //ROOT_ID means the call came from engine c++ rather than squirrel.
                if(edge.callerId == ProfileReport::ROOT_ID){
                    entry.AddMember("caller", rapidjson::Value("engine", allocator), allocator);
                }else{
                    entry.AddMember("caller", edge.callerId, allocator);
                }
                entry.AddMember("callee", edge.calleeId, allocator);
                entry.AddMember("calls", edge.callCount, allocator);
                entry.AddMember("inclusiveMs", edge.inclusiveMs, allocator);
                edges.PushBack(entry, allocator);
            }
            doc.AddMember("edges", edges, allocator);
        }

        if(!report.hotLines.empty()){
            rapidjson::Value lines(rapidjson::kArrayType);
            for(const ProfileReport::Line& line : report.hotLines){
                rapidjson::Value entry(rapidjson::kObjectType);
                entry.AddMember("function", line.functionId, allocator);
                entry.AddMember("name", rapidjson::Value(line.functionName.c_str(), allocator), allocator);
                entry.AddMember("source", rapidjson::Value(line.source.c_str(), allocator), allocator);
                entry.AddMember("line", line.line, allocator);
                entry.AddMember("executions", line.executions, allocator);
                entry.AddMember("selfMs", line.selfMs, allocator);
                entry.AddMember("pctSelf", line.pctSelf, allocator);
                lines.PushBack(entry, allocator);
            }
            doc.AddMember("hotLines", lines, allocator);
        }

        if(!report.frames.empty()){
            rapidjson::Value frames(rapidjson::kArrayType);
            for(const ProfileReport::Frame& frame : report.frames){
                rapidjson::Value entry(rapidjson::kObjectType);
                entry.AddMember("frame", frame.frameIndex, allocator);
                entry.AddMember("scriptMs", frame.scriptMs, allocator);
                entry.AddMember("calls", frame.calls, allocator);
                entry.AddMember("topRootMs", frame.topRootMs, allocator);
                if(frame.topRootFunctionId != ProfileReport::ROOT_ID){
                    entry.AddMember("topRootFunction", frame.topRootFunctionId, allocator);
                }
                frames.PushBack(entry, allocator);
            }
            doc.AddMember("frames", frames, allocator);
        }
    }
}

#endif
