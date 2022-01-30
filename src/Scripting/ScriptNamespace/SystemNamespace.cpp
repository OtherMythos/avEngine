#include "SystemNamespace.h"

#include "System/Util/PathUtils.h"
#include "filesystem/path.h"

namespace AV{

    SQInteger SystemNamespace::makeDirectory(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);
        //TODO make this only able to read and write in the saves directory.

        bool result = filesystem::create_directory(outString);
        if(!result) return sq_throwerror(vm, "Error creating directory.");

        return 0;
    }

    /**SQNamespace
    @name _system
    @desc Functions relating to the underlying system.
    */
    void SystemNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name mkdir
        @desc Create a directory at the specified path.
        */
        ScriptUtils::addFunction(vm, makeDirectory, "mkdir", 2, ".s");
    }
}
