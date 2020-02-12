#include "MiscFunctions.h"

#include "World/Support/OgreMeshManager.h"
#include "Scripting/ScriptNamespace/Classes/MeshClass.h"
#include "System/BaseSingleton.h"

#include <time.h>
#include "sqstdio.h"

namespace AV{

    SQInteger MiscFunctions::doFile(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, -1, &filePath);

        sq_pop(vm, 1); //Pop the string so we have access to the underlying context.

        sqstd_dofile(vm, filePath, false, true);

        return 0;
    }

    SQInteger MiscFunctions::getTime(HSQUIRRELVM vm){
        SQInteger t = (SQInteger)time(NULL);
        sq_pushinteger(vm, t);
        return 1;
    }

    void MiscFunctions::setupFunctions(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, doFile, "_doFile", 2, ".s");
        ScriptUtils::addFunction(vm, getTime, "_time");
    }
}
