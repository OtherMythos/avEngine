#ifdef SCRIPT_PROFILER

#pragma once

#include <squirrel.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "ProfileReport.h"

namespace AV{

    struct ScriptProfilerSettings{
        //Per line execution counts and self time. This is the most expensive collector,
        //because unlike the others it does work on every executed line rather than on
        //every call, so it is worth turning off when measuring something call heavy.
        bool collectLines = true;
        bool collectEdges = true;
        bool collectFrames = true;
        //Where to write the full report at shutdown. Empty logs a summary only.
        std::string outputPath;
    };

    /**
    Measures squirrel execution: how often each function is called, how long it takes
    inclusive and exclusive of its callees, which function called it, which frames were
    expensive, and which source lines the vm spends its time on.

    Works by consuming the vm's debug hook through SquirrelHookDispatcher. Squirrel emits
    a 'c' event on entering a squirrel closure and an 'r' event on leaving it; those
    balance exactly across tail calls, exception unwinding and generator yield/resume, so
    a shadow stack of open calls is enough to time them.

    Two consequences of how squirrel reports this are worth knowing when reading a report:

    - Native (c++) closures never fire the hook, so engine namespace functions are
      invisible. Their cost lands in the *exclusive* time of the squirrel function that
      called them.
    - Because ScriptVM enables debug info, the hook also fires on every executed source
      line. That is the floor cost of profiling, and it is what the per line timing uses.

    Compiled in behind SCRIPT_PROFILER and enabled at runtime with --profileScripts.
    Main thread only, like the vm it measures.
    */
    class ScriptProfiler{
    public:
        ScriptProfiler() = delete;
        ~ScriptProfiler() = delete;

        /**
        Install the hook and begin collecting. Call once, on the main vm.
        */
        static void initialise(HSQUIRRELVM vm, const ScriptProfilerSettings& settings);

        /**
        Write the configured report, release the strings pinned in the vm and detach.
        Must run while the vm is still open.
        */
        static void shutdown();

        //Collection can be paused and resumed without losing what has been gathered.
        static void start();
        static void stop();
        /**
        Clear all counters, keeping the functions already discovered. This is how a
        caller skips engine startup and profiles only what happens next.
        */
        static void reset();

        static bool isEnabled() { return mEnabled; }
        static bool isRunning() { return mRunning; }

        /**
        Close off the current frame's timeline entry. Called once per frame by Base.
        */
        static void notifyFrameBoundary();

        static void buildReport(ProfileReport& out, const ProfileQuery& query);

        /**
        Write a full report to a file.
        @param json True for json, false for the text table.
        @return An empty string on success, otherwise the reason it failed.
        */
        static std::string writeReportToFile(const std::string& path, bool json);

        /**
        True when the path ends in .json, i.e. the caller wants json output.
        */
        static bool pathWantsJson(const std::string& path);

        typedef uint64_t(*ClockFunction)();

        /**
        Test seam: replace the nanosecond clock so timings are deterministic, and feed
        hook events without a running vm. Passing a null vm to initialise skips the
        string pinning, which is the only part that needs one.
        */
        static void _setClockForTesting(ClockFunction func);
        static void _hookForTesting(SQInteger type, const SQChar* source, SQInteger line, const SQChar* funcName);

    private:
        //Identifies a call entered from engine c++ rather than from squirrel.
        static const uint32_t ROOT_INDEX = ProfileReport::ROOT_ID;
        //Frames of timeline history kept, about ten seconds at 60fps.
        static const size_t FRAME_HISTORY = 600;

        /**
        Identity of a squirrel function, as the hook reports it.

        Squirrel interns every string, so while a string is alive its address uniquely
        identifies it and this can be compared without touching memory. Keeping the
        strings alive is what makes that safe; see _pinString.
        */
        struct FuncKey{
            const SQChar* source;
            const SQChar* name;
            SQInteger declLine;

            bool operator==(const FuncKey& other) const{
                return source == other.source && name == other.name && declLine == other.declLine;
            }
        };

        struct FuncKeyHash{
            size_t operator()(const FuncKey& key) const{
                size_t hash = reinterpret_cast<size_t>(key.source);
                hash ^= reinterpret_cast<size_t>(key.name) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                hash ^= static_cast<size_t>(key.declLine) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                return hash;
            }
        };

        struct FunctionRecord{
            std::string source;
            std::string name;
            int declLine = 0;

            uint64_t callCount = 0;
            uint64_t inclusiveNs = 0;
            uint64_t exclusiveNs = 0;
            //0 means unset; no completed call can take zero nanoseconds to observe.
            uint64_t minInclusiveNs = 0;
            uint64_t maxInclusiveNs = 0;
            uint32_t maxStackDepth = 0;

            //Live state rather than a statistic: how many frames of this function are open.
            uint32_t activeCount = 0;

            HSQOBJECT sourceObj;
            HSQOBJECT nameObj;
            bool pinned = false;
        };

        struct StackFrame{
            uint32_t funcIndex;
            uint64_t startNs;
            //Inclusive time of everything this frame has called so far.
            uint64_t childNs;
        };

        struct EdgeRecord{
            uint64_t callCount = 0;
            uint64_t inclusiveNs = 0;
        };

        struct LineRecord{
            uint64_t executions = 0;
            //Time spent on this line excluding squirrel functions it called, but
            //including any native engine function it called.
            uint64_t selfNs = 0;
        };

        struct FrameEntry{
            uint64_t frameIndex = 0;
            uint64_t scriptNs = 0;
            uint32_t calls = 0;
            uint32_t topRootFunc = ROOT_INDEX;
            uint64_t topRootNs = 0;
        };

        static HSQUIRRELVM mVm;
        static ScriptProfilerSettings mSettings;
        static ClockFunction mClock;
        static bool mEnabled;
        static bool mRunning;

        static std::vector<FunctionRecord> mFunctions;
        static std::unordered_map<FuncKey, uint32_t, FuncKeyHash> mIndexByPointer;
        //Fallback identity, so a script reload which rebuilds the interned strings
        //merges back into the existing record instead of reporting a duplicate.
        static std::unordered_map<std::string, uint32_t> mIndexByName;

        static std::vector<StackFrame> mStack;
        //Keyed (caller << 32) | callee.
        static std::unordered_map<uint64_t, EdgeRecord> mEdges;
        //Keyed (function << 32) | line.
        static std::unordered_map<uint64_t, LineRecord> mLines;

        static std::vector<FrameEntry> mFrameHistory;
        static uint64_t mFrameHistoryCount;

        static uint64_t mStartNs;
        static uint64_t mStoppedNs;
        static uint64_t mTotalScriptNs;
        static uint64_t mTotalCalls;
        static uint64_t mFramesElapsed;
        static uint64_t mLineExecutions;

        //The line currently being timed, and when it started. Held as a pointer because
        //unordered_map never invalidates element addresses, which saves a second lookup
        //on the hottest path in the profiler. Null between activations: it is cleared on
        //every call and return, so that time spent in a callee, and the idle gap between
        //frames, is never charged to whichever line happened to run last.
        static LineRecord* mOpenLine;
        static uint64_t mOpenLineNs;

        static uint64_t mFrameScriptNs;
        static uint32_t mFrameCalls;
        static uint32_t mFrameTopRootFunc;
        static uint64_t mFrameTopRootNs;

        static void _hook(HSQUIRRELVM vm, SQInteger type, const SQChar* sourceName, SQInteger line, const SQChar* funcName);

        static void _onCall(HSQUIRRELVM vm, const SQChar* source, SQInteger line, const SQChar* funcName);
        static void _onReturn();
        static void _onLine(SQInteger line);
        /**
        Charge the time since it started to the line currently being timed, and stop
        timing it.
        */
        static void _closeOpenLine();

        static uint32_t _resolveFunction(HSQUIRRELVM vm, const FuncKey& key);
        static uint32_t _createFunction(HSQUIRRELVM vm, const FuncKey& key);
        /**
        Keep an interned squirrel string alive for the profiler's lifetime.

        This is what makes the pointer keyed lookup correct: a released string could
        otherwise have its address reused by an unrelated one, silently merging two
        functions' statistics.
        */
        static void _pinString(HSQUIRRELVM vm, const SQChar* str, HSQOBJECT& out);
        static void _releasePinnedStrings();

        static void _clearCounters();
        static double _elapsedMs();
    };
}

#endif
