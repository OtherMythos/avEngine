#include "DebugNamespace.h"
#ifdef DEBUG_SERVER
#include "System/Base.h"
#include "System/BaseSingleton.h"
#include "System/SystemSetup/SystemSettings.h"

namespace AV{
    SQInteger DebugNamespace::pauseSimulation(HSQUIRRELVM vm){
        if(!SystemSettings::isDebugServerEnabled()) return sq_throwerror(vm, "pauseSimulation requires --debugServer");
        const SQChar* reason;
        sq_getstring(vm, 2, &reason);
        if(sq_getsize(vm, 2) > 1024) return sq_throwerror(vm, "pause reason exceeds 1024 bytes");
        BaseSingleton::getBase()->pauseSimulation(reason);
        return 0;
    }

    void DebugNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, pauseSimulation, "pauseSimulation", 2, ".s");
    }
}
#endif
