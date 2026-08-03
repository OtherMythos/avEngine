#pragma once

#include <squirrel.h>

namespace AV{
    class ScriptWorker;

    /**
    Creates the reduced virtual machine a script worker runs in.

    The main vm registers 33 namespaces and around 45 delegate tables, almost all of which reach
    the scene graph, the entity system, the gui or Ogre - none of which may be touched from a
    worker thread. Rather than trying to make those safe, a worker vm gets a deliberately small
    root table: the squirrel standard libraries, file loading, json, a per worker random source
    and _workerSelf. Data goes in, data comes out.

    No userdata types are registered at all. Every delegate table in the engine is held in a
    single file static bound to the main vm, so registering one here would mean either sharing
    that static across vms or duplicating the type. Neither is worth it for a vm which is meant
    to be moving plain values around. If one is ever genuinely needed, add it to create() - the
    point of this file is that there is one place to add it.

    Anything registered here MUST be safe to run on an arbitrary thread while the main thread is
    running game logic. In practice that means: no engine singletons, no Ogre, no global mutable
    state. ScriptWorkerVMTests asserts the contents of the root table so this does not quietly
    erode.
    */
    class ScriptWorkerVM{
    public:
        ScriptWorkerVM() = delete;

        /**
        Open and populate a worker vm.
        @param owner Stored as the vm's foreign pointer so the error handler and the namespaces
                     registered here can find their worker without a file static. May be null,
                     which is only useful for tests inspecting the root table.
        */
        static HSQUIRRELVM create(ScriptWorker* owner);

        /**
        The worker which owns this vm, or null.
        */
        static ScriptWorker* getOwner(HSQUIRRELVM vm);
    };
}
