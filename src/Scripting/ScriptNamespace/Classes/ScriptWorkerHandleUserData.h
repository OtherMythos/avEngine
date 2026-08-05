#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/Worker/ScriptWorker.h"

namespace AV{
    class ScriptWorkerManager;

    /**
    The script side handle to a worker thread, as returned by _worker.create().

    The handle stores the worker's id rather than a pointer, so a handle kept past a destroy()
    reports itself as invalid instead of dangling. There is deliberately no release hook: unlike a
    Timer, a worker has a thread attached to it, and release hooks run in unspecified order inside
    the main vm's sq_close. Worker lifetime belongs to ScriptWorkerManager; destroy() is explicit
    and anything a script forgets is reaped at shutdown.
    */
    class ScriptWorkerHandleUserData{
    public:
        ScriptWorkerHandleUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void workerIdToUserData(HSQUIRRELVM vm, ScriptWorkerId id);
        static UserDataGetResult readWorkerIdFromUserData(HSQUIRRELVM vm, SQInteger stackInx, ScriptWorkerId* outId);

    private:
        static SQObject workerDelegateTableObject;

        static SQInteger dispatch(HSQUIRRELVM vm);
        static SQInteger poll(HSQUIRRELVM vm);
        static SQInteger claim(HSQUIRRELVM vm);
        static SQInteger progress(HSQUIRRELVM vm);
        static SQInteger error(HSQUIRRELVM vm);
        static SQInteger destroy(HSQUIRRELVM vm);
        static SQInteger workerToString(HSQUIRRELVM vm);

        /**
        The worker this handle points at, or null if it has been destroyed. Every method starts here.
        */
        static ScriptWorker* _resolve(HSQUIRRELVM vm, ScriptWorkerManager** outManager);
    };
}
