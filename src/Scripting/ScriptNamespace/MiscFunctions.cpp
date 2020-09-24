#include "MiscFunctions.h"

#include "World/Support/OgreMeshManager.h"
#include "Scripting/ScriptNamespace/Classes/MeshClass.h"
#include "System/BaseSingleton.h"
#include "Window/SDL2Window/SDL2Window.h"

#include <time.h>
#include "sqstdio.h"

#include "System/Util/PathUtils.h"

namespace AV{

    SQInteger MiscFunctions::doFile(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, -1, &filePath);

        //Optimisation. I'm sure there would be ways to improve this, as I'm doing a lot of passing strings around right now.
        //For instance, rather than creating that outString on the stack I could just create it somewhere else.
        //In future this could be replaced with a pure c string approach, and given how other functions will be using the res:// paths that should be worth the time.
        std::string outString;
        formatResToPath(filePath, outString);

        sq_pop(vm, 1); //Pop the string so we have access to the underlying context.

        if(!fileExists(outString)){
            std::string s("Script at path does not exist: ");
            s += outString;
            return sq_throwerror(vm, s.c_str());
        }

        if(SQ_FAILED(sqstd_dofile(vm, outString.c_str(), false, true))){
            return sq_throwerror(vm, "Error executing script file.");
        }

        return 0;
    }

    SQInteger MiscFunctions::getTime(HSQUIRRELVM vm){
        SQInteger t = (SQInteger)time(NULL);
        sq_pushinteger(vm, t);
        return 1;
    }

    SQInteger MiscFunctions::shutdownEngine(HSQUIRRELVM vm){
        Window* win = BaseSingleton::getWindow();
        SDL2Window* sdlWin = static_cast<SDL2Window*>(win);
        //TODO this should be changed at some point.
        sdlWin->wantsToClose = true;

        return 0;
    }

    void MiscFunctions::setupFunctions(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, doFile, "_doFile", 2, ".s");
        ScriptUtils::addFunction(vm, getTime, "_time");
        ScriptUtils::addFunction(vm, shutdownEngine, "_shutdownEngine");
    }
}
