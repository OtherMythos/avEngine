#if defined(DEBUG_SERVER) && defined(SCRIPT_PROFILER)

#include "ProfilerInspector.h"

#include "Scripting/Profiler/ScriptProfiler.h"

namespace AV{
    namespace{
        //Compiled in but not collecting is a normal state, not a client error, so the
        //response says so and names the flag that would change it.
        bool writeDisabled(rapidjson::Document& doc){
            if(ScriptProfiler::isEnabled()) return false;

            rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
            doc.SetObject();
            doc.AddMember("enabled", false, allocator);
            doc.AddMember("hint", rapidjson::Value(
                "the script profiler is compiled in but was not enabled; relaunch the engine with --profileScripts",
                allocator), allocator);
            return true;
        }
    }

    void ProfilerInspector::writeProfile(rapidjson::Document& doc, int& status, const ProfileQuery& query){
        if(writeDisabled(doc)) return;

        ProfileReport report;
        ScriptProfiler::buildReport(report, query);
        ProfileReportWriter::toJson(report, doc);

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.AddMember("sort", rapidjson::Value(ProfileQuery::sortName(query.sort), allocator), allocator);
        status = 200;
    }

    void ProfilerInspector::writeFunctionDetail(rapidjson::Document& doc, int& status, uint32_t functionId){
        if(writeDisabled(doc)) return;

        ProfileQuery query;
        query.singleFunction = true;
        query.functionId = functionId;
        query.maxFunctions = 0;
        query.includeEdges = true;
        query.includeLines = true;
        query.maxEdges = 0;
        query.maxLines = 40;

        ProfileReport report;
        ScriptProfiler::buildReport(report, query);

        if(report.functions.empty()){
            rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
            doc.SetObject();
            doc.AddMember("error", rapidjson::Value(
                ("no profiled function with id " + std::to_string(functionId)).c_str(), allocator), allocator);
            status = 404;
            return;
        }

        //The edges came back filtered to this function, but a caller wants to know which
        //side of each edge it sits on. Resolving the other end's name here saves them a
        //second request per edge.
        ProfileQuery namesQuery;
        namesQuery.maxFunctions = 0;
        ProfileReport allFunctions;
        ScriptProfiler::buildReport(allFunctions, namesQuery);

        ProfileReportWriter::toJson(report, doc);
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value callers(rapidjson::kArrayType);
        rapidjson::Value callees(rapidjson::kArrayType);
        for(const ProfileReport::Edge& edge : report.edges){
            const bool isCaller = edge.calleeId == functionId;
            const uint32_t otherId = isCaller ? edge.callerId : edge.calleeId;

            rapidjson::Value entry(rapidjson::kObjectType);
            if(otherId == ProfileReport::ROOT_ID){
                entry.AddMember("name", rapidjson::Value("<engine>", allocator), allocator);
            }else{
                const ProfileReport::Function* other = allFunctions.findFunction(otherId);
                entry.AddMember("id", otherId, allocator);
                if(other){
                    entry.AddMember("name", rapidjson::Value(other->name.c_str(), allocator), allocator);
                    entry.AddMember("source", rapidjson::Value(other->source.c_str(), allocator), allocator);
                    entry.AddMember("line", other->declLine, allocator);
                }
            }
            entry.AddMember("calls", edge.callCount, allocator);
            entry.AddMember("inclusiveMs", edge.inclusiveMs, allocator);

            if(isCaller) callers.PushBack(entry, allocator);
            else callees.PushBack(entry, allocator);
        }
        doc.AddMember("callers", callers, allocator);
        doc.AddMember("callees", callees, allocator);
        //The raw edge list is now redundant with callers/callees.
        doc.RemoveMember("edges");

        status = 200;
    }

    void ProfilerInspector::writeControl(rapidjson::Document& doc, int& status, Control control){
        if(!ScriptProfiler::isEnabled()){
            writeDisabled(doc);
            //Unlike a read, an action the engine cannot perform should not look like success.
            status = 409;
            return;
        }

        const char* action = "";
        switch(control){
            case Control::START: ScriptProfiler::start(); action = "start"; break;
            case Control::STOP: ScriptProfiler::stop(); action = "stop"; break;
            case Control::RESET: ScriptProfiler::reset(); action = "reset"; break;
        }

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("enabled", true, allocator);
        doc.AddMember("action", rapidjson::Value(action, allocator), allocator);
        doc.AddMember("running", ScriptProfiler::isRunning(), allocator);
        status = 200;
    }

    void ProfilerInspector::writeDump(rapidjson::Document& doc, int& status, const std::string& path, bool json){
        if(!ScriptProfiler::isEnabled()){
            writeDisabled(doc);
            status = 409;
            return;
        }

        const std::string error = ScriptProfiler::writeReportToFile(path, json);

        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();
        if(error.empty()){
            doc.AddMember("written", rapidjson::Value(path.c_str(), allocator), allocator);
            doc.AddMember("format", rapidjson::Value(json ? "json" : "text", allocator), allocator);
            status = 200;
        }else{
            doc.AddMember("error", rapidjson::Value(("could not write " + path + ": " + error).c_str(), allocator), allocator);
            status = 500;
        }
    }
}

#endif
