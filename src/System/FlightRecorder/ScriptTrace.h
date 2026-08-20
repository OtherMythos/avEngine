#ifdef FLIGHT_RECORDER

#pragma once

#include <squirrel.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "ScriptEventRing.h"

namespace AV{

    struct FrameRecord;
    struct RecorderSettings;

    /**
    One entry of the interned function table the events index into.
    */
    struct TracedFunction{
        std::string source;
        std::string name;
        int declLine = 0;
    };

    /**
    One frame of a live squirrel backtrace, with its local variables.
    */
    struct BacktraceFrame{
        std::string function;
        std::string source;
        int line = 0;
        std::vector<std::pair<std::string, std::string>> locals;
    };

    /**
    Records what the squirrel vm did, as a consumer of the shared debug hook.

    Only call ('c') and return ('r') events are handled; line ('l') events are ignored
    because they fire on every executed line and are the profiler's dominant cost. What
    remains is cheap enough to leave running: an index lookup and a fixed size write per call.

    Function identity uses the same trick as ScriptProfiler - squirrel interns its strings, so
    while a string is alive its address identifies it - and pins those strings so the
    addresses stay valid.

    Main thread only, like the vm it observes.
    */
    class ScriptTrace{
    public:
        ScriptTrace() = delete;
        ~ScriptTrace() = delete;

        /**
        Register as a hook consumer and begin recording. Call once, on the main vm.
        */
        static void initialise(HSQUIRRELVM vm, const RecorderSettings& settings);

        /**
        Release the pinned strings and detach. Must run while the vm is still open.
        */
        static void shutdown();

        static void start();
        static void stop();
        static bool isRunning() { return mRunning; }

        /**
        Close off the frame: write the frame's event slice, deepest stack, marks and watch
        values into the record. Runs once per rendered frame, on the main thread.
        */
        static void closeFrame(FrameRecord& out);

        /**
        Annotate the frame currently being recorded.
        */
        static void mark(const std::string& tag);

        /**
        Register a closure to be called once per frame, its return value stringified into that
        frame's record. Replaces any watch of the same name.
        */
        static void watch(const std::string& name, HSQOBJECT closure);
        static bool unwatch(const std::string& name);
        static std::vector<std::string> watchNames();

        /**
        Walk the vm's live call stack, collecting each frame's function, source line and
        locals.

        Only meaningful while script code is executing - from a script triggered capture or
        the error handler. Called when the vm is idle (an F3 capture) it correctly returns
        empty, and the frame records' deepest stacks are the evidence to use instead.
        */
        static std::vector<BacktraceFrame> buildBacktrace();

        /**
        The interned function table the recorded events index into.
        */
        static const std::vector<TracedFunction>& functions() { return mFunctions; }

        /**
        Copy out the events belonging to a frame's slice, clamped to what the ring still holds.
        */
        static std::vector<ScriptEvent> sliceEvents(uint64_t begin, uint64_t end);

        /**
        Stringify the squirrel value on top of the stack, bounded in both depth and length.

        ScriptUtils::_getStringForType recurses without limit and would not survive a cyclic
        or very deep game table, which a watch closure can easily return.
        */
        static std::string stringifyValue(HSQUIRRELVM vm, int maxDepth = 3);

        //Test seam: feed hook events without a running vm.
        static void _hookForTesting(SQInteger type, const SQChar* source, SQInteger line, const SQChar* funcName);

    private:
        static void _hook(HSQUIRRELVM vm, SQInteger type, const SQChar* sourceName, SQInteger line, const SQChar* funcName);

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

        struct WatchEntry{
            std::string name;
            HSQOBJECT closure;
        };

        static uint32_t _resolveFunction(HSQUIRRELVM vm, const FuncKey& key);
        static void _pinString(HSQUIRRELVM vm, const SQChar* str, HSQOBJECT& out);
        static void _releasePinned();
        static void _evaluateWatches(FrameRecord& out);
        static void _stringifyRecursive(HSQUIRRELVM vm, int depth, int maxDepth, std::string& out);

        static uint64_t _now();

        static HSQUIRRELVM mVm;
        static bool mEnabled;
        static bool mRunning;

        static ScriptEventRing mRing;
        static std::vector<TracedFunction> mFunctions;
        //Interned string pointers pinned so their addresses stay unique and valid.
        static std::vector<std::pair<HSQOBJECT, HSQOBJECT>> mPinned;
        static std::unordered_map<FuncKey, uint32_t, FuncKeyHash> mIndexByPointer;
        static std::unordered_map<std::string, uint32_t> mIndexByName;

        static std::vector<WatchEntry> mWatches;
        static std::vector<std::string> mFrameMarks;
        //Absolute event index at which the frame being recorded began.
        static uint64_t mFrameEventBegin;

        //Cap on how much text a single watch value may contribute.
        static const size_t MAX_VALUE_LENGTH = 2048;
        //Cap on entries expanded from one table or array.
        static const size_t MAX_CONTAINER_ENTRIES = 32;
    };
}

#endif
