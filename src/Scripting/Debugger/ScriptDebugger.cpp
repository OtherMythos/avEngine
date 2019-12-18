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
        : _sqvm(vm) {

        debugger = this;

        _determineBreakpoints();
    }

    ScriptDebugger::~ScriptDebugger(){

    }

    void ScriptDebugger::_debugHook(HSQUIRRELVM vm, SQInteger type, const SQChar *sourceName, SQInteger line, const SQChar *funcName){
        //TODO maybe move this to a function within the ScriptDebugger instance rather than this static function.
        if(!debugger->mCurrentlyDebugging){
            //If we're not in a debugging state, check if any of the breakpoints have been triggered.
            int id = 0;
            for(const ScriptDebugger::BreakpointInfo& b : debugger->mBreakpoints){
                if(line != b.line) continue; //Check the line first as that's much more efficient than checking the path each time.
                if(sourceName != b.filePath) continue;

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
        if(mBreakpoints.size() > 0){

            if(!mHookSet){
                //Set the debug hook.
                sq_setnativedebughook(_sqvm, _debugHook);
                mHookSet = true;
            }

        }else{
            //The breakpoints might be cleared while debugging is active. In this case don't disable the hook.
            if(mHookSet && !mCurrentlyDebugging){
                //Set the debug hook.
                sq_setnativedebughook(_sqvm, NULL);
                mHookSet = false;
            }

        }
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
        _printCurrentFrame();
        bool debuggerLoop = true;

        while(debuggerLoop){

            std::cout << ">>> ";

            std::string strIn;
            std::getline(std::cin, strIn);

            //TODO make this enter thing work.
            if(strIn == "\n"){
                //i.e the enter key was just pressed as it is.
                strIn = previousCommand;
            }

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
                clearAllBreakpoints();
            }

            else if(strIn == "frame"){
                _printCurrentFrame();
            }
            else if(strIn == "backtrace"){
                _printBacktrace();
            }

            if(strIn.rfind("p ") == 0){
                //If the string starts with "p " then it's trying to print out a local variable.
                const std::string targetVariable = strIn.substr(2);
                _printLocalVariableByName(targetVariable);
            }

            previousCommand = strIn;
        }
    }

    void ScriptDebugger::_printCurrentFrame(){
        SQStackInfos si;
        sq_stackinfos(_sqvm, 0, &si);

        AV_WARN("");
        AV_WARN("Function: {}", si.funcname);
        AV_WARN("    {}:{}", si.source, si.line);

        const std::string codeLine = _readLineFromFile(si.source, si.line);
        AV_WARN(codeLine);
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
        while((name = sq_getlocal(_sqvm, 0, seq))) {

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


        const SQChar *name = NULL;
        int seq=0;
        while((name = sq_getlocal(_sqvm, 0, seq))) {

            if(targetVariableName == name){
                std::string outStr;
                _getStringForType(_sqvm, outStr);

                AV_WARN("\"{}\" = {}", name, outStr);

                sq_pop(_sqvm, 1); //Pop the value as we're going to return.
                return;
            }
            sq_pop(_sqvm, 1);

            seq++;
        }

        AV_WARN("No variable named {}", targetVariableName);
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
