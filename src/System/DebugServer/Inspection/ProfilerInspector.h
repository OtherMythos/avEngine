#if defined(DEBUG_SERVER) && defined(SCRIPT_PROFILER)

#pragma once

#include <string>
#include <rapidjson/document.h>

#include "Scripting/Profiler/ProfileReport.h"

namespace AV{

    /**
    Serves the script profiler over the debug server.

    Every call here reads ScriptProfiler state, which is mutated from inside the vm's
    debug hook, so all of it must run on the main thread through MainThreadQueue.
    */
    class ProfilerInspector{
    public:
        ProfilerInspector() = delete;
        ~ProfilerInspector() = delete;

        /**
        Status plus whichever sections the query asked for.
        */
        static void writeProfile(rapidjson::Document& doc, int& status, const ProfileQuery& query);

        /**
        One function in full: its callers, its callees and its hottest lines. The way to
        drill in without paying for the whole report.
        */
        static void writeFunctionDetail(rapidjson::Document& doc, int& status, uint32_t functionId);

        enum class Control{
            START,
            STOP,
            RESET
        };
        static void writeControl(rapidjson::Document& doc, int& status, Control control);

        /**
        Write the full report to a path on disk, so bulk data never has to travel back
        through the response.
        */
        static void writeDump(rapidjson::Document& doc, int& status, const std::string& path, bool json);
    };
}

#endif
