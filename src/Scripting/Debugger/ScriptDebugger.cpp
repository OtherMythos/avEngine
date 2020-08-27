#ifdef DEBUGGING_TOOLS

#include "ScriptDebugger.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include "System/SystemSetup/SystemSettings.h"
#include "filesystem/path.h"
#include <regex>

#include "Logger/Log.h"

namespace AV{
    static ScriptDebugger* debugger = 0;

    ScriptDebugger::ScriptDebugger(HSQUIRRELVM vm)
        : _sqvm(vm),
          mCurrentDebugFrame(0) {

        debugger = this;

        _determineBreakpoints();
    }

    ScriptDebugger::~ScriptDebugger(){

    }

    void ScriptDebugger::_debugHook(HSQUIRRELVM vm, SQInteger type, const SQChar *sourceName, SQInteger line, const SQChar *funcName){
        if(!debugger->mCurrentlyDebugging){
            //If we're not in a debugging state, check if any of the breakpoints have been triggered.
            int id = 0;
            for(const ScriptDebugger::BreakpointInfo& b : debugger->mBreakpoints){
                if(line != b.line) continue; //Check the line first as that's much more efficient than checking the path each time.
                if(sourceName != b.filePath) continue;

                debugger->mFirstDebuggingFrame = false;
                AV_WARN("Hit breakpoint {}", id);
                AV_WARN("Line {} of file {}", line, sourceName);
                id++;

                debugger->_beginDebugging();
            }
        }

        //Check the debugging state again. We might have landed in it after the previous check.
        if(debugger->mCurrentlyDebugging){
            debugger->_updateDebuggerLogic();
        }
    }

    void ScriptDebugger::registerBreakpoint(const std::string& filePath, int lineNum){
        mBreakpoints.push_back({filePath, lineNum});

        _updateHook();
    }

    void ScriptDebugger::deleteBreakpoint(const std::string& filePath, int lineNum){
        auto it = debugger->mBreakpoints.begin();
        while(it != debugger->mBreakpoints.end()){
            const ScriptDebugger::BreakpointInfo& b = *it;
            if(b.line == b.line && b.filePath == filePath){
                debugger->mBreakpoints.erase(it);
            }else it++;
        }
    }

    void ScriptDebugger::clearAllBreakpoints(){
        mBreakpoints.clear();
        _updateHook();
    }

    void ScriptDebugger::_updateHook(){
        if(mBreakpoints.size() > 0 || mCurrentlyDebugging){

            if(!mHookSet){
                //Set the debug hook.
                _setHook(true);
            }

        }else{
            //The breakpoints might be cleared while debugging is active. In this case don't disable the hook.
            if(mHookSet && !mCurrentlyDebugging){
                //Set the debug hook.
                _setHook(false);
            }

        }
    }

    void ScriptDebugger::_setHook(bool set){
        sq_setnativedebughook(_sqvm, set ? _debugHook : NULL);
        mHookSet = set;
    }

    void ScriptDebugger::beginDebugging(){
        AV_WARN("Welcome to the avEngine squirrel debugger.");
        AV_WARN("Type 'help' for more information.");

        mFirstDebuggingFrame = true;
        _beginDebugging();
        _setHook(true);
        debugger->_updateDebuggerLogic();
    }

    void ScriptDebugger::_beginDebugging(){
        mCurrentlyDebugging = true;
    }

    void ScriptDebugger::_endDebugging(){
        mCurrentlyDebugging = false;

        //All the breakpoints might have been deleted while debugging, in which case the hook would remain active.
        //If we're now ending debugging we should check whether the hook needs updating.
        _updateHook();
    }

    void ScriptDebugger::_updateDebuggerLogic(){
        if(!mFirstDebuggingFrame){
            _printCurrentFrame();
        }
        bool debuggerLoop = true;

        while(debuggerLoop){

            std::cout << ">>> ";

            std::string strIn;
            if(!std::getline(std::cin, strIn)){
                //The chances are the user has pressed ctrl-D or ctrl-C, so exit the debugger.
                //Print a new line to make up for the fact that nothing was typed.
                std::cout << "\n";
                _exitAndClose();
            }

            if(strIn.empty()){
                //i.e the enter key was just pressed as it is.
                strIn = previousCommand;
            }

            static const std::regex frameRegex("^frame \\d+$");

            //Define some functionality very similar to gdb.
            if(strIn == "n"){
                debuggerLoop = false;
            }
            else if(strIn == "c"){
                debuggerLoop = false;
                _endDebugging();
            }
            else if(strIn == "p" || strIn == "info locals"){
                _printLocalVariables();
            }
            else if(strIn == "d"){
                AV_WARN("Removed {} breakpoints.", mBreakpoints.size());
                clearAllBreakpoints();
            }

            else if(strIn == "exit"){
                _exitAndClose();
            }

            else if(strIn == "frame"){
                _printCurrentFrame();
            }
            else if(strIn == "list"){
                _printDescriptiveFrame();
            }
            else if(strIn == "backtrace"){
                _printBacktrace();
            }

            else if(strIn == "help"){
                _printHelp();
            }
            else if(std::regex_match(strIn, frameRegex)){
                std::string newString = strIn.substr(6, strIn.length());
                int targetFrame = std::stoi(newString);
                //The user is specifying a stack frame to switch to.
                _switchToFrame(targetFrame);
            }

            if(strIn.rfind("p ") == 0){
                //If the string starts with "p " then it's trying to print out a local variable.
                const std::string targetVariable = strIn.substr(2);
                _printLocalVariableByName(targetVariable);
            }

            previousCommand = strIn;
        }

        mFirstDebuggingFrame = false;
    }

    void ScriptDebugger::_exitAndClose(){
        AV_WARN("Exiting the debugger and closing the engine.");
        AV_WARN("Have a nice day!");
        _endDebugging();

        exit(0); //While it would be better to have a proper shutdown, I don't have a problem with this right now.
    }

    void ScriptDebugger::_printHelp(){
        AV_WARN("n - step the program.");
        AV_WARN("c - continue execution.");
        AV_WARN("p, info locals - print local variables.");
        AV_WARN("d - delete all breakpoints.");
        AV_WARN("exit - exit the engine.");
        AV_WARN("frame - print the current stack frame.");
        AV_WARN("backtrace - print the backtrace.");
        AV_WARN("list - print the current line of code with some surrounding lines.");
        AV_WARN("p [varName] - print a specific variable.");
        AV_WARN("p root - print the root table.");
    }

    void ScriptDebugger::_switchToFrame(int frame){
        SQStackInfos si;
        if(SQ_SUCCEEDED(sq_stackinfos(_sqvm, frame, &si))){
            mCurrentDebugFrame = frame;
            AV_WARN("Switching to frame {}", mCurrentDebugFrame);

            _printCurrentFrame();
        }else{
            AV_ERROR("The current debug stack does not contain a frame {}.", frame);
        }
    }

    void ScriptDebugger::_printCurrentFrame(){
        SQStackInfos si;
        sq_stackinfos(_sqvm, mCurrentDebugFrame, &si);

        _printCurrentFrame(si.funcname, si.source, si.line);

        const std::string codeLine = _readLineFromFile(si.source, si.line);
        AV_WARN(codeLine);
    }

    void ScriptDebugger::_printCurrentFrame(const char* funcname, const char* sourceFile, int line){
        AV_WARN("");
        AV_WARN("Function: {}", funcname);
        AV_WARN("    {}:{}", sourceFile, line);
    }

    void ScriptDebugger::_printDescriptiveFrame(){
        SQStackInfos si;
        sq_stackinfos(_sqvm, mCurrentDebugFrame, &si);

        _printCurrentFrame();

        AV_WARN("===============================");

        _printRegionOfFile(si.source, si.line, 10);
    }

    void ScriptDebugger::_printBacktrace(){
        SQStackInfos si;
        int c = 0;
        AV_WARN("====BACKTRACE====");
        while(SQ_SUCCEEDED(sq_stackinfos(_sqvm, c, &si))){
            AV_WARN("#{} {} at {}:{}", c, si.funcname, si.source, si.line);
            c++;
        }
        AV_WARN("=================");
    }

    void ScriptDebugger::_printLocalVariables(){
        AV_WARN("====LOCAL VARIABLES====");

        const SQChar *name = NULL;
        int seq=0;
        while((name = sq_getlocal(_sqvm, mCurrentDebugFrame, seq))) {

            std::string outStr;
            _getStringForType(_sqvm, outStr);

            AV_WARN("\"{}\" = {}", name, outStr);


            sq_pop(_sqvm, 1);
            seq++;
        }
        AV_WARN("=======================");
    }

    void ScriptDebugger::_printLocalVariableByName(const std::string& targetVariableName){
        if(targetVariableName == "root"){
            //In this case print the root table.
            sq_pushroottable(_sqvm);

            std::string outStr;
            _getStringForType(_sqvm, outStr);

            AV_WARN("\"{}\" = {}", targetVariableName, outStr);

            sq_pop(_sqvm, 1); //Pop the root table.
            return;
        }

        bool isTargetRoot = targetVariableName.rfind("::", 0) == 0;
        //Remove the :: from the string if it was provided like that.
        const std::string rawValName = isTargetRoot ? targetVariableName.substr(2, targetVariableName.size() - 2) : targetVariableName;

        if(!isTargetRoot){
            const SQChar *name = NULL;
            int seq=0;
            while((name = sq_getlocal(_sqvm, mCurrentDebugFrame, seq))) {

                if(rawValName == name){
                    std::string outStr;
                    _getStringForType(_sqvm, outStr);

                    AV_WARN("\"{}\" = {}", name, outStr);

                    sq_pop(_sqvm, 1); //Pop the value as we're going to return.
                    return;
                }
                sq_pop(_sqvm, 1);

                seq++;
            }
        }

        //Check if the root table contains the variable name.
        {
            //In this case print the root table.
            sq_pushroottable(_sqvm);

            sq_pushnull(_sqvm);
            while(SQ_SUCCEEDED(sq_next(_sqvm, -2))){
                const SQChar *name;
                sq_getstring(_sqvm, -2, &name);

                if(rawValName == name){
                    std::string outStr;
                    _getStringForType(_sqvm, outStr);

                    //I leave this as the full name so its clear to the user.
                    AV_WARN("\"{}\" = {}", targetVariableName, outStr);

                    sq_pop(_sqvm, 4); //Pop the value, key null iterator and root table.
                    return;
                }

                sq_pop(_sqvm,2); //pop the key and value
            }
            sq_pop(_sqvm,2); //pops the null iterator and root table.
        }

        AV_WARN("No variable named '{}'", targetVariableName);
    }

    void ScriptDebugger::_getStringForType(HSQUIRRELVM vm, std::string& outStr){

        auto t = sq_gettype(_sqvm,-1);
        if(t == OT_TABLE){
            std::ostringstream stream;
            _iterateTable(_sqvm, stream);

            outStr = stream.str();
        }
        else if(t == OT_ARRAY){
            std::ostringstream stream;
            _iterateArray(_sqvm, stream);

            outStr = stream.str();
        }else{
            sq_tostring(_sqvm, -1);
            const SQChar* sqStr;
            sq_getstring(_sqvm, -1, &sqStr);
            sq_pop(_sqvm, 1);

            if(t == OT_STRING){
                outStr = "\"" + std::string(sqStr) + "\"";
            }else outStr = sqStr;
        }
    }

    void ScriptDebugger::_iterateArray(HSQUIRRELVM vm, std::ostringstream& stream){
        stream << "[";

        assert(sq_gettype(vm,-1) == OT_ARRAY);

        bool previousValue = false;
        sq_pushnull(vm);  //null iterator
        while(SQ_SUCCEEDED(sq_next(vm, -2))){
            if(previousValue){
                stream << ", ";
            }
            previousValue = true;

            std::string outStr;
            _getStringForType(vm, outStr);

            stream << outStr;

            //This does push a string, it's just not needed.
            sq_pop(vm, 2);
        }

        sq_pop(vm, 1); //pop the null iterator.

        stream << "]";
    }

    void ScriptDebugger::_iterateTable(HSQUIRRELVM vm, std::ostringstream& stream){
        stream << "{";

        assert(sq_gettype(vm,-1) == OT_TABLE);

        bool previousValue = false;
        sq_pushnull(vm);  //null iterator
        while(SQ_SUCCEEDED(sq_next(vm, -2))){
            if(previousValue){
                stream << ", ";
            }
            previousValue = true;

            const SQChar *key;
            sq_getstring(vm, -2, &key);

            std::string outStr;
            _getStringForType(vm, outStr);

            stream << "\"" << key << "\"";
            stream << " : " << outStr;

            sq_pop(vm, 2);
        }

        sq_pop(vm, 1); //pop the null iterator. The table is popped during the sq_getlocal section.

        stream << "}";
    }

    std::string ScriptDebugger::_readLineFromFile(const char* filePath, int lineNumber){
        //Here I open and read the file each time I need to get a line.
        //This isn't the fastest thing in the world, but it's fine for a non-performance critical debugger.
        int lineCount = 1;
        std::string line;
        std::ifstream file(filePath);
        if (file.is_open()){
            while (!file.eof()){
                getline(file,line);
                if(lineCount == lineNumber) {
                    file.close();
                    return line;
                }
                lineCount++;
            }
            file.close();
        }

        return "<Could not find line in file>";
    }

    void ScriptDebugger::_printRegionOfFile(const char* filePath, int middleLineNum, int padding){
        int startLine = middleLineNum - padding;
        int endLine = middleLineNum + padding + 1; //Add 1 for the current line.
        if(startLine < 0) startLine = 0;

        std::string line;
        std::ifstream file(filePath);
        int currentLine = 1;
        if(file.is_open()){
            while(!file.eof()){
                getline(file,line);

                if(currentLine >= startLine && currentLine < endLine){
                    if(currentLine == middleLineNum){
                        AV_WARN("{} >>> {}", currentLine, line);
                    }else{
                        AV_WARN("{}     {}", currentLine, line);
                    }
                }
                currentLine++;
            }
            file.close();
        }
    }

    void ScriptDebugger::_determineBreakpoints(){
        const std::string& path = SystemSettings::getMasterPath();
        const filesystem::path targetPath(filesystem::path(path) / filesystem::path("breakpoints"));

        if(!targetPath.exists() || !targetPath.is_file()) return;

        std::string line;
        std::ifstream myfile(targetPath.str());
        if (myfile.is_open()){
            while(getline(myfile,line)){
                static const std::regex e("(.)*:\\d(\\d)*$"); //Something of the form 'path:lineNum'
                if(!std::regex_match(line, e)) continue;

                size_t foundIndex = line.find_last_of(":");
                const std::string filePath = line.substr(0, foundIndex);
                int lineNum = std::stoi(line.substr(foundIndex + 1));

                registerBreakpoint(filePath, lineNum);
            }
        }else{
            AV_ERROR("Error opening the breakpoints file.")
        }
    }
}

#endif
