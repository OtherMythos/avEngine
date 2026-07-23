#ifdef SCRIPT_PROFILER

#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <rapidjson/document.h>

namespace AV{

    /**
    A flat, self contained snapshot of what ScriptProfiler has collected.

    Deliberately free of any dependency on the vm, Ogre or the debug server, so the
    sorting, derived maths and serialisation can be unit tested on hand built data.
    */
    struct ProfileReport{
        //Identifies a call which entered squirrel from engine c++ rather than from
        //another squirrel function.
        static const uint32_t ROOT_ID = 0xFFFFFFFF;

        struct Function{
            uint32_t id = 0;
            std::string source;
            std::string name;
            int declLine = 0;
            uint64_t callCount = 0;
            double inclusiveMs = 0.0;
            double exclusiveMs = 0.0;
            //Average and extremes are of inclusive time, per call.
            double avgUs = 0.0;
            double minUs = 0.0;
            double maxUs = 0.0;
            //Share of all profiled script time, 0-100.
            double pctExclusive = 0.0;
            double callsPerFrame = 0.0;
            uint32_t maxStackDepth = 0;
        };

        struct Edge{
            uint32_t callerId = ROOT_ID;
            uint32_t calleeId = 0;
            uint64_t callCount = 0;
            double inclusiveMs = 0.0;
        };

        struct Line{
            uint32_t functionId = 0;
            //Carried inline rather than looked up by id, so a line entry can be read on
            //its own even when the response's function table is capped.
            std::string functionName;
            std::string source;
            int line = 0;
            //How many times the line ran, and how long it held the vm excluding any
            //squirrel function it called.
            uint64_t executions = 0;
            double selfMs = 0.0;
            //Share of all measured line time, 0-100.
            double pctSelf = 0.0;
        };

        struct Frame{
            uint64_t frameIndex = 0;
            double scriptMs = 0.0;
            uint32_t calls = 0;
            //The single most expensive root call in this frame, i.e. the entry point
            //to blame for a spike.
            uint32_t topRootFunctionId = ROOT_ID;
            double topRootMs = 0.0;
        };

        struct Totals{
            bool enabled = false;
            bool running = false;
            //Wall time the profiler has been collecting for.
            double elapsedMs = 0.0;
            //Of that, time spent inside squirrel root calls.
            double scriptMs = 0.0;
            uint64_t totalCalls = 0;
            uint64_t framesElapsed = 0;
            uint64_t lineExecutions = 0;
            uint32_t functionsSeen = 0;
            //How many functions the query limit left out of this report.
            uint32_t functionsOmitted = 0;
        };

        Totals totals;
        std::vector<Function> functions;
        std::vector<Edge> edges;
        std::vector<Line> hotLines;
        std::vector<Frame> frames;

        /**
        Resolve an id to its entry, or 0 if the query limit left it out.
        */
        const Function* findFunction(uint32_t id) const;
    };

    /**
    What a caller wants out of a report. The defaults are the small ones, suitable for
    an agent's context budget; the file dump raises the limits.
    */
    struct ProfileQuery{
        enum class Sort{
            EXCLUSIVE,
            INCLUSIVE,
            CALLS,
            AVERAGE
        };

        Sort sort = Sort::EXCLUSIVE;
        //Cap on functions returned. 0 means no limit.
        uint32_t maxFunctions = 25;
        //Drop functions called fewer times than this.
        uint64_t minCalls = 0;

        bool includeEdges = false;
        bool includeLines = false;
        bool includeFrames = false;
        uint32_t maxEdges = 100;
        uint32_t maxLines = 50;
        uint32_t maxFrames = 60;
        //Order frames by script time rather than chronologically.
        bool worstFramesFirst = false;

        //Restrict the report to one function, plus the edges and lines touching it.
        bool singleFunction = false;
        uint32_t functionId = 0;

        /**
        Parse a sort name as accepted by the debug server. Returns false if unrecognised,
        leaving out untouched.
        */
        static bool parseSort(const std::string& str, Sort& out);
        static const char* sortName(Sort sort);
    };

    namespace ProfileReportWriter{
        /**
        A fixed width table. Used for the shutdown log summary and .txt dumps.
        */
        std::string toText(const ProfileReport& report);

        /**
        The same content as json. Used for the debug server responses and .json dumps.
        */
        void toJson(const ProfileReport& report, rapidjson::Document& doc);
    }
}

#endif
