#include "ScriptDebugger.h"

#include <cassert>
#include <iostream>

#include "Logger/Log.h"

namespace AV{
    static ScriptDebugger* debugger = 0;

    ScriptDebugger::ScriptDebugger(HSQUIRRELVM vm)
        : _sqvm(vm) {

        debugger = this;

        //TODO temporary
        registerBreakpoint("/home/edward/Documents/avData/simpleTextures/squirrelEntry.nut", 26);
        //deleteBreakpoint("/home/edward/Documents/avData/simpleTextures/squirrelEntry.nut", 26);
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
            std::cin >> strIn;

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
            else if(strIn == "p"){
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

            previousCommand = strIn;
        }
    }

    void ScriptDebugger::_printCurrentFrame(){
        SQStackInfos si;
        sq_stackinfos(_sqvm, 0, &si);

        AV_WARN("");
        AV_WARN("Function: {}", si.funcname);
        AV_WARN("    {}:{}", si.source, si.line);
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
        const SQChar *name = NULL;
        int seq=0;
        while((name = sq_getlocal(_sqvm, 0, seq))) {
            AV_WARN(name);

            sq_pop(_sqvm, 1);
            seq++;
        }
    }
}
