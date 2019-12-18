#pragma once

#include <string>
#include <squirrel.h>
#include <vector>
#include <sstream>

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

        void _printLocalVariableByName(const std::string& targetVariableName);

        std::string _readLineFromFile(const char* filePath, int lineNumber);

        /**
        Get a stringified version of a squirrel variable's value.
        If this item is a table or array it will be expanded to show its contents.

        @remarks
        This function expects the item at the top of the stack to be that which needs to be converted into a string.
        */
        void _getStringForType(HSQUIRRELVM vm, std::string& outStr);
        void _iterateArray(HSQUIRRELVM vm, std::ostringstream& stream);
        void _iterateTable(HSQUIRRELVM vm, std::ostringstream& stream);
    };
}
