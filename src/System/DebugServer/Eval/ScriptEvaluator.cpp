#ifdef DEBUG_SERVER

#include "ScriptEvaluator.h"

#include "Logger/Log.h"

#include <vector>
#include <cstdarg>
#include <cstdio>

namespace AV{
    //Print capture. Eval runs one at a time on the main thread (serialised by the
    //MainThreadQueue pump), so plain file-scope state is safe here.
    static std::vector<std::string>* sPrintSink = nullptr;

    static void capturePrintFunc(HSQUIRRELVM vm, const SQChar* s, ...){
        char buffer[1024];
        va_list args;
        va_start(args, s);
        vsnprintf(buffer, sizeof(buffer), s, args);
        va_end(args);

        if(sPrintSink) sPrintSink->push_back(buffer);
        //Keep forensics in the log as well: an eval that breaks the engine should
        //leave a trace beyond the HTTP response.
        AV_SQUIRREL_PRINT("(eval) {}", buffer);
    }

    static std::string lastErrorString(HSQUIRRELVM vm){
        std::string result = "unknown error";
        const SQInteger top = sq_gettop(vm);
        sq_getlasterror(vm);
        if(SQ_SUCCEEDED(sq_tostring(vm, -1))){
            const SQChar* str = nullptr;
            if(SQ_SUCCEEDED(sq_getstring(vm, -1, &str)) && str){
                result = str;
            }
        }
        sq_settop(vm, top);
        return result;
    }

    static const char* typeName(SQObjectType type){
        switch(type){
            case OT_USERDATA: return "userdata";
            case OT_CLOSURE: return "closure";
            case OT_NATIVECLOSURE: return "nativeclosure";
            case OT_GENERATOR: return "generator";
            case OT_USERPOINTER: return "userpointer";
            case OT_THREAD: return "thread";
            case OT_CLASS: return "class";
            case OT_INSTANCE: return "instance";
            case OT_WEAKREF: return "weakref";
            default: return "unknown";
        }
    }

    static const int MAX_SERIALISE_DEPTH = 8;

    //Serialise the value at idx (absolute index) into out. Tables and arrays recurse
    //with a depth cap so cyclic structures cannot hang the engine.
    static void writeStackValue(HSQUIRRELVM vm, SQInteger idx, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator, int depth){
        const SQObjectType type = sq_gettype(vm, idx);
        switch(type){
            case OT_NULL:
                out.SetNull();
                break;
            case OT_BOOL:{
                SQBool val;
                sq_getbool(vm, idx, &val);
                out.SetBool(val == SQTrue);
                break;
            }
            case OT_INTEGER:{
                SQInteger val;
                sq_getinteger(vm, idx, &val);
                out.SetInt64(static_cast<int64_t>(val));
                break;
            }
            case OT_FLOAT:{
                SQFloat val;
                sq_getfloat(vm, idx, &val);
                out.SetDouble(static_cast<double>(val));
                break;
            }
            case OT_STRING:{
                const SQChar* str = nullptr;
                sq_getstring(vm, idx, &str);
                out.SetString(str ? str : "", allocator);
                break;
            }
            case OT_ARRAY:{
                if(depth >= MAX_SERIALISE_DEPTH){
                    out.SetString("<max depth reached>", allocator);
                    break;
                }
                out.SetArray();
                sq_pushnull(vm); //Iterator.
                while(SQ_SUCCEEDED(sq_next(vm, idx))){
                    rapidjson::Value entry;
                    writeStackValue(vm, sq_gettop(vm), entry, allocator, depth + 1);
                    out.PushBack(entry, allocator);
                    sq_pop(vm, 2); //Key and value.
                }
                sq_pop(vm, 1); //Iterator.
                break;
            }
            case OT_TABLE:{
                if(depth >= MAX_SERIALISE_DEPTH){
                    out.SetString("<max depth reached>", allocator);
                    break;
                }
                out.SetObject();
                sq_pushnull(vm);
                while(SQ_SUCCEEDED(sq_next(vm, idx))){
                    const SQInteger valueIdx = sq_gettop(vm);
                    const SQInteger keyIdx = valueIdx - 1;

                    //Keys become strings for JSON; non-string keys go through tostring.
                    std::string keyString;
                    if(sq_gettype(vm, keyIdx) == OT_STRING){
                        const SQChar* str = nullptr;
                        sq_getstring(vm, keyIdx, &str);
                        if(str) keyString = str;
                    }else{
                        if(SQ_SUCCEEDED(sq_tostring(vm, keyIdx))){
                            const SQChar* str = nullptr;
                            if(SQ_SUCCEEDED(sq_getstring(vm, -1, &str)) && str) keyString = str;
                            sq_pop(vm, 1);
                        }
                    }

                    rapidjson::Value key(keyString.c_str(), allocator);
                    rapidjson::Value entry;
                    writeStackValue(vm, valueIdx, entry, allocator, depth + 1);
                    out.AddMember(key, entry, allocator);
                    sq_pop(vm, 2);
                }
                sq_pop(vm, 1);
                break;
            }
            default:{
                //Userdata, instances, closures etc: report the type and its tostring
                //representation rather than pretending it is null.
                out.SetObject();
                out.AddMember("type", rapidjson::Value(typeName(type), allocator), allocator);
                std::string repr;
                if(SQ_SUCCEEDED(sq_tostring(vm, idx))){
                    const SQChar* str = nullptr;
                    if(SQ_SUCCEEDED(sq_getstring(vm, -1, &str)) && str) repr = str;
                    sq_pop(vm, 1);
                }
                out.AddMember("tostring", rapidjson::Value(repr.c_str(), allocator), allocator);
                break;
            }
        }
    }

    void ScriptEvaluator::eval(HSQUIRRELVM vm, const std::string& code, rapidjson::Document& doc){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        const SQInteger top = sq_gettop(vm);

        //Capture prints for the response, forwarding to the log as usual.
        std::vector<std::string> prints;
        SQPRINTFUNCTION oldPrint = sq_getprintfunc(vm);
        SQPRINTFUNCTION oldError = sq_geterrorfunc(vm);
        sPrintSink = &prints;
        sq_setprintfunc(vm, capturePrintFunc, oldError);

        //raiseerror stays SQFalse throughout: errors are read from sq_getlasterror, and
        //the VM's installed handlers (which raise test-failure events under TEST_MODE)
        //are never invoked for agent snippets.
        bool compiled = false;
        std::string compileError;
        {
            //Expression-first, so "1+1" just works.
            const std::string expr = "return (" + code + ");";
            if(SQ_SUCCEEDED(sq_compilebuffer(vm, expr.c_str(), expr.size(), "eval", SQFalse))){
                compiled = true;
            }else if(SQ_SUCCEEDED(sq_compilebuffer(vm, code.c_str(), code.size(), "eval", SQFalse))){
                compiled = true;
            }else{
                compileError = lastErrorString(vm);
            }
        }

        bool ok = false;
        std::string error;
        if(!compiled){
            error = "compile error: " + compileError;
        }else{
            sq_pushroottable(vm);
            if(SQ_FAILED(sq_call(vm, 1, SQTrue, SQFalse))){
                error = "runtime error: " + lastErrorString(vm);
            }else{
                rapidjson::Value result;
                writeStackValue(vm, sq_gettop(vm), result, allocator, 0);
                doc.AddMember("result", result, allocator);
                ok = true;
            }
        }

        //Restore print routing and the stack no matter what happened above.
        sq_setprintfunc(vm, oldPrint, oldError);
        sPrintSink = nullptr;
        sq_settop(vm, top);

        doc.AddMember("ok", ok, allocator);
        if(!ok){
            doc.AddMember("error", rapidjson::Value(error.c_str(), allocator), allocator);
        }
        rapidjson::Value printsValue(rapidjson::kArrayType);
        for(const std::string& line : prints){
            printsValue.PushBack(rapidjson::Value(line.c_str(), allocator), allocator);
        }
        doc.AddMember("prints", printsValue, allocator);
    }
}

#endif
