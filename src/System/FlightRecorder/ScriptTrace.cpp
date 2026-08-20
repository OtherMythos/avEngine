#ifdef FLIGHT_RECORDER

#include "ScriptTrace.h"

#include "FrameRing.h"
#include "RecorderSettings.h"

#include "Scripting/SquirrelHookDispatcher.h"
#include "Scripting/ScriptVM.h"
#include "Logger/Log.h"

#include <chrono>

namespace AV{

    HSQUIRRELVM ScriptTrace::mVm = 0;
    bool ScriptTrace::mEnabled = false;
    bool ScriptTrace::mRunning = false;

    ScriptEventRing ScriptTrace::mRing;
    std::vector<TracedFunction> ScriptTrace::mFunctions;
    std::vector<std::pair<HSQOBJECT, HSQOBJECT>> ScriptTrace::mPinned;
    std::unordered_map<ScriptTrace::FuncKey, uint32_t, ScriptTrace::FuncKeyHash> ScriptTrace::mIndexByPointer;
    std::unordered_map<std::string, uint32_t> ScriptTrace::mIndexByName;

    std::vector<ScriptTrace::WatchEntry> ScriptTrace::mWatches;
    std::vector<std::string> ScriptTrace::mFrameMarks;
    uint64_t ScriptTrace::mFrameEventBegin = 0;

    static const char* UNKNOWN_SOURCE = "unknown";
    static const char* ANONYMOUS_FUNCTION = "anonymous";

    uint64_t ScriptTrace::_now(){
        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    void ScriptTrace::initialise(HSQUIRRELVM vm, const RecorderSettings& settings){
        mVm = vm;
        mRing.configure(settings.scriptEvents);
        mFunctions.clear();
        mPinned.clear();
        mIndexByPointer.clear();
        mIndexByName.clear();
        mFrameMarks.clear();
        mFrameEventBegin = 0;

        mEnabled = true;
        mRunning = true;

        SquirrelHookDispatcher::setConsumer(SquirrelHookDispatcher::Consumer::RECORDER, &ScriptTrace::_hook);
    }

    void ScriptTrace::shutdown(){
        if(!mEnabled) return;

        SquirrelHookDispatcher::setConsumer(SquirrelHookDispatcher::Consumer::RECORDER, 0);

        _releasePinned();

        //Watch closures were referenced when registered; release them while the vm lives.
        if(mVm){
            for(WatchEntry& w : mWatches){
                sq_release(mVm, &w.closure);
            }
        }
        mWatches.clear();

        mRing.clear();
        mFunctions.clear();
        mIndexByPointer.clear();
        mIndexByName.clear();
        mFrameMarks.clear();

        mEnabled = false;
        mRunning = false;
        mVm = 0;
    }

    void ScriptTrace::start(){ if(mEnabled) mRunning = true; }
    void ScriptTrace::stop(){ mRunning = false; }

    void ScriptTrace::_hook(HSQUIRRELVM vm, SQInteger type, const SQChar* sourceName, SQInteger line, const SQChar* funcName){
        if(!mRunning) return;

        switch(type){
            case _SC('c'):{
                const FuncKey key{ sourceName, funcName, line };
                mRing.pushCall(_resolveFunction(vm, key), static_cast<int32_t>(line), _now());
                break;
            }
            case _SC('r'):
                mRing.pushReturn(static_cast<int32_t>(line), _now());
                break;
            //Line events fire on every executed line. Ignored: they are the profiler's
            //dominant cost and add nothing the call trace does not already say.
            default: break;
        }
    }

    void ScriptTrace::_hookForTesting(SQInteger type, const SQChar* source, SQInteger line, const SQChar* funcName){
        _hook(0, type, source, line, funcName);
    }

    uint32_t ScriptTrace::_resolveFunction(HSQUIRRELVM vm, const FuncKey& key){
        auto it = mIndexByPointer.find(key);
        if(it != mIndexByPointer.end()) return it->second;

        const std::string source = key.source ? key.source : UNKNOWN_SOURCE;
        const std::string name = key.name ? key.name : ANONYMOUS_FUNCTION;
        const std::string mergeKey = source + "\n" + name + "\n" + std::to_string(static_cast<long long>(key.declLine));

        auto nameIt = mIndexByName.find(mergeKey);
        if(nameIt != mIndexByName.end()){
            //Same function, fresh interned strings after a script reload.
            mIndexByPointer[key] = nameIt->second;
            return nameIt->second;
        }

        const uint32_t index = static_cast<uint32_t>(mFunctions.size());
        mFunctions.push_back(TracedFunction{ source, name, static_cast<int>(key.declLine) });

        if(vm){
            HSQOBJECT sourceObj;
            HSQOBJECT nameObj;
            sq_resetobject(&sourceObj);
            sq_resetobject(&nameObj);
            if(key.source) _pinString(vm, key.source, sourceObj);
            if(key.name) _pinString(vm, key.name, nameObj);
            mPinned.push_back({ sourceObj, nameObj });
        }

        mIndexByName[mergeKey] = index;
        mIndexByPointer[key] = index;
        return index;
    }

    void ScriptTrace::_pinString(HSQUIRRELVM vm, const SQChar* str, HSQOBJECT& out){
        //Safe inside the hook: squirrel disables it for the duration of the callback, and
        //the stack is restored before returning.
        const SQInteger top = sq_gettop(vm);
        sq_pushstring(vm, str, -1);
        sq_getstackobj(vm, -1, &out);
        sq_addref(vm, &out);
        sq_settop(vm, top);
    }

    void ScriptTrace::_releasePinned(){
        if(mVm){
            for(std::pair<HSQOBJECT, HSQOBJECT>& p : mPinned){
                if(!sq_isnull(p.first)) sq_release(mVm, &p.first);
                if(!sq_isnull(p.second)) sq_release(mVm, &p.second);
            }
        }
        mPinned.clear();
    }

    void ScriptTrace::closeFrame(FrameRecord& out){
        out.scriptEventBegin = mFrameEventBegin;
        out.scriptEventEnd = mRing.nextIndex();
        out.deepestStack = mRing.takeDeepestStack();
        out.marks = mFrameMarks;

        _evaluateWatches(out);

        mFrameMarks.clear();
        mFrameEventBegin = mRing.nextIndex();
    }

    void ScriptTrace::mark(const std::string& tag){
        if(!mEnabled) return;
        mFrameMarks.push_back(tag);
    }

    void ScriptTrace::watch(const std::string& name, HSQOBJECT closure){
        if(!mEnabled || !mVm) return;

        unwatch(name);

        WatchEntry entry;
        entry.name = name;
        entry.closure = closure;
        sq_addref(mVm, &entry.closure);
        mWatches.push_back(entry);
    }

    bool ScriptTrace::unwatch(const std::string& name){
        for(auto it = mWatches.begin(); it != mWatches.end(); ++it){
            if(it->name != name) continue;
            if(mVm) sq_release(mVm, &it->closure);
            mWatches.erase(it);
            return true;
        }
        return false;
    }

    std::vector<std::string> ScriptTrace::watchNames(){
        std::vector<std::string> out;
        out.reserve(mWatches.size());
        for(const WatchEntry& w : mWatches) out.push_back(w.name);
        return out;
    }

    void ScriptTrace::_evaluateWatches(FrameRecord& out){
        if(!mVm || mWatches.empty()) return;

        for(const WatchEntry& w : mWatches){
            WatchValue value;
            value.name = w.name;

            //Calling a watch closure re-enters the vm, which would record the call as part of
            //the next frame's trace and pollute it. Pause collection for the duration.
            const bool wasRunning = mRunning;
            mRunning = false;

            const SQInteger top = sq_gettop(mVm);
            sq_pushobject(mVm, w.closure);
            sq_pushroottable(mVm);
            if(SQ_SUCCEEDED(sq_call(mVm, 1, SQTrue, SQTrue))){
                value.value = stringifyValue(mVm);
            }else{
                const SQChar* err = 0;
                sq_getlasterror(mVm);
                if(SQ_SUCCEEDED(sq_getstring(mVm, -1, &err)) && err){
                    value.value = std::string("<error: ") + err + ">";
                }else{
                    value.value = "<error>";
                }
            }
            sq_settop(mVm, top);

            mRunning = wasRunning;

            out.watches.push_back(value);
        }
    }

    std::string ScriptTrace::stringifyValue(HSQUIRRELVM vm, int maxDepth){
        std::string out;
        _stringifyRecursive(vm, 0, maxDepth, out);
        if(out.size() > MAX_VALUE_LENGTH){
            out.resize(MAX_VALUE_LENGTH);
            out += "...<truncated>";
        }
        return out;
    }

    void ScriptTrace::_stringifyRecursive(HSQUIRRELVM vm, int depth, int maxDepth, std::string& out){
        const SQObjectType type = sq_gettype(vm, -1);

        if(type == OT_TABLE || type == OT_ARRAY){
            if(depth >= maxDepth){
                //Bounded here rather than by trusting the data: a game table can be cyclic,
                //and the engine's own stringifier recurses until it runs out of stack.
                out += (type == OT_TABLE) ? "{...}" : "[...]";
                return;
            }

            const bool isTable = (type == OT_TABLE);
            out += isTable ? "{" : "[";

            size_t entries = 0;
            bool truncated = false;
            const SQInteger top = sq_gettop(vm);
            sq_pushnull(vm);
            while(SQ_SUCCEEDED(sq_next(vm, -2))){
                if(entries >= MAX_CONTAINER_ENTRIES){
                    truncated = true;
                    sq_pop(vm, 2);
                    break;
                }
                if(entries > 0) out += ", ";

                if(isTable){
                    //Key sits below the value; stringify it without descending.
                    sq_push(vm, -2);
                    sq_tostring(vm, -1);
                    const SQChar* keyStr = 0;
                    if(SQ_SUCCEEDED(sq_getstring(vm, -1, &keyStr)) && keyStr){
                        out += keyStr;
                        out += ": ";
                    }
                    sq_pop(vm, 2);
                }

                _stringifyRecursive(vm, depth + 1, maxDepth, out);
                sq_pop(vm, 2);
                entries++;

                if(out.size() > MAX_VALUE_LENGTH){
                    truncated = true;
                    sq_pop(vm, 1);
                    break;
                }
            }
            //Restore whatever the iteration left behind, including an early break.
            sq_settop(vm, top);

            if(truncated) out += ", ...";
            out += isTable ? "}" : "]";
            return;
        }

        sq_tostring(vm, -1);
        const SQChar* str = 0;
        if(SQ_SUCCEEDED(sq_getstring(vm, -1, &str)) && str){
            if(type == OT_STRING){
                out += "\"";
                out += str;
                out += "\"";
            }else{
                out += str;
            }
        }else{
            out += "<unknown>";
        }
        sq_pop(vm, 1);
    }

    std::vector<BacktraceFrame> ScriptTrace::buildBacktrace(){
        std::vector<BacktraceFrame> out;
        if(!mVm) return out;

        //Frame 0 is whatever called us, so start at 1 to describe the script itself. When the
        //vm is idle there are no frames at all and this loop does not run.
        SQStackInfos si;
        for(SQInteger frame = 1; SQ_SUCCEEDED(sq_stackinfos(mVm, frame, &si)); frame++){
            BacktraceFrame f;
            f.function = si.funcname ? si.funcname : ANONYMOUS_FUNCTION;
            f.source = si.source ? si.source : UNKNOWN_SOURCE;
            f.line = static_cast<int>(si.line);

            const SQChar* name = 0;
            SQUnsignedInteger seq = 0;
            while((name = sq_getlocal(mVm, static_cast<SQUnsignedInteger>(frame), seq))){
                f.locals.push_back({ name, stringifyValue(mVm) });
                sq_pop(mVm, 1);
                seq++;
            }

            out.push_back(f);
        }

        return out;
    }

    std::vector<ScriptEvent> ScriptTrace::sliceEvents(uint64_t begin, uint64_t end){
        return mRing.slice(begin, end);
    }
}

#endif
