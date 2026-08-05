#include "ScriptWorkerVM.h"

#include "ScriptWorker.h"
#include "WorkerNamespace/WorkerRandomNamespace.h"
#include "WorkerNamespace/WorkerSelfNamespace.h"

#include "Scripting/ScriptVM.h"
#include "Scripting/ScriptVMShared.h"
#include "Scripting/ScriptNamespace/MiscFunctions.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/SystemNamespace.h"

#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdstring.h>

namespace AV{

    namespace{
        SQInteger workerErrorHandler(HSQUIRRELVM vm){
            std::string message;
            sqLogRuntimeError(vm, "worker", &message);

            //The vm knows which worker owns it, so a single handler serves all of them.
            ScriptWorker* worker = ScriptWorkerVM::getOwner(vm);
            if(worker) worker->recordError(message);

            //Deliberately no debugger attach and no test mode failure event. A worker script
            //failing is reported through the handle's error(), and must never take the engine
            //down or fail a test run the way a main vm failure does.
            return 0;
        }

        void workerCompilerError(HSQUIRRELVM vm, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column){
            sqLogCompilerError("worker", desc, source, line, column);

            ScriptWorker* worker = ScriptWorkerVM::getOwner(vm);
            if(worker){
                std::string message(desc ? desc : "compilation error");
                if(source){
                    message += " (";
                    message += source;
                    message += ":" + std::to_string(static_cast<long long>(line)) + ")";
                }
                worker->recordError(message);
            }
        }
    }

    ScriptWorker* ScriptWorkerVM::getOwner(HSQUIRRELVM vm){
        return static_cast<ScriptWorker*>(sq_getforeignptr(vm));
    }

    HSQUIRRELVM ScriptWorkerVM::create(ScriptWorker* owner){
        HSQUIRRELVM vm = sq_open(1024);

        sq_setforeignptr(vm, owner);
        sq_setprintfunc(vm, sqPrintFunc, NULL);
        //Without this the error handler reports a meaningless source and line.
        sq_enabledebuginfo(vm, true);

        sq_newclosure(vm, workerErrorHandler, 0);
        sq_seterrorhandler(vm);
        sq_setcompilererrorhandler(vm, workerCompilerError);

        sq_pushroottable(vm);

        sqstd_register_mathlib(vm);
        sqstd_register_stringlib(vm);
        sqstd_register_bloblib(vm);

        ScriptVM::setupNamespace(vm, "_workerSelf", WorkerSelfNamespace::setupNamespace);
        ScriptVM::setupNamespace(vm, "_random", WorkerRandomNamespace::setupNamespace);
        ScriptVM::setupNamespace(vm, "_system", SystemNamespace::setupWorkerNamespace);

        MiscFunctions::setupWorkerFunctions(vm);

        //Mirrors the existing EXECUTION_SETUP_VM flag, so a file shared between the main vm and a
        //worker can tell which one it has been loaded into.
        ScriptUtils::declareConstant(vm, "EXECUTION_SETUP_VM", 0);
        ScriptUtils::declareConstant(vm, "EXECUTION_WORKER_VM", 1);

        WorkerSelfNamespace::setupConstants(vm);

        sq_pop(vm, 1); //The root table.

        return vm;
    }
}
