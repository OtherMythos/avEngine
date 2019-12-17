#pragma once

#include <string>
#include <squirrel.h>
#include <vector>

namespace AV{

    /**
    Debugging facilities for squirrel scripts.
    Allows the insertion of break points, inspection of code, etc.
    */
    class ScriptDebugger{
    public:
        ScriptDebugger(HSQUIRRELVM vm);
        ~ScriptDebugger();

        /**
        Register a breakpoint with the debugger.

        @param filePath
        An absolute path to the specific file. If the path is not absolute there might be problems triggering the breakpoint.
        */
        void registerBreakpoint(const std::string& filePath, int lineNum);

        void deleteBreakpoint(const std::string& filePath, int lineNum);
        void clearAllBreakpoints();

        struct BreakpointInfo{
            std::string filePath;
            int line;

            bool operator < ( const BreakpointInfo &_r ) const {
                return _r.line < line && _r.filePath < filePath;
            }
        };

    private:
        HSQUIRRELVM _sqvm;

        std::vector<BreakpointInfo> mBreakpoints;

        bool mHookSet = false;
        bool mCurrentlyDebugging = false;
        std::string previousCommand = "n"; //n for default because it's normally the most useful.

        static void _debugHook(HSQUIRRELVM vm, SQInteger type, const SQChar *sourceName, SQInteger line, const SQChar *funcName);

        /**
        Update the vm debug hook, depending on whether or not there are any breakpoints or not.
        */
        void _updateHook();

        void _beginDebugging();
        void _endDebugging();
        //Called to process logic related to debugging, for instance checking vm state.
        void _updateDebuggerLogic();

        void _printCurrentFrame();
        void _printBacktrace();
        void _printLocalVariables();
    };
}
