#include "ScriptVMShared.h"

#include "Logger/Log.h"

#include <cstdarg>
#include <cstdio>

namespace AV{

    void sqPrintFunc(HSQUIRRELVM v, const SQChar *s, ...){
        char buffer[256];
        va_list args;
        va_start (args, s);
        vsnprintf (buffer, 256, s, args);
        va_end (args);

        AV_SQUIRREL_PRINT("{}", buffer);
    }

    void sqLogRuntimeError(HSQUIRRELVM vm, const char* vmTag, std::string* outMessage, SQStackInfos* outStackInfo){
        const SQChar* sqErr = 0;
        sq_getlasterror(vm);
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        sq_pop(vm, 1);

        SQStackInfos si;
        sq_stackinfos(vm, 1, &si);

        static const std::string separator(10, '=');

        AV_ERROR(separator);

        AV_ERROR("[{}] Error during script execution.", vmTag);
        AV_ERROR(sqErr);
        AV_ERROR("In file {}", si.source);
        AV_ERROR("    on line {}", si.line);
        AV_ERROR("of function {}", si.funcname);

        AV_ERROR(separator);

        if(outMessage) *outMessage = sqErr ? sqErr : "unknown script error";
        if(outStackInfo) *outStackInfo = si;
    }

    void sqLogCompilerError(const char* vmTag, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column){
        static const std::string separator(10, '=');

        AV_ERROR(separator);

        AV_ERROR("[{}] Error during script compilation.", vmTag);
        AV_ERROR(desc);
        AV_ERROR("In file {}", source);
        AV_ERROR("    on line {} column {}", line, column);

        AV_ERROR(separator);
    }
}
