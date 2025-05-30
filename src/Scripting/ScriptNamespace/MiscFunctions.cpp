#include "MiscFunctions.h"

#include "World/Support/OgreMeshManager.h"
#include "Scripting/ScriptNamespace/Classes/MeshClass.h"
#include "System/BaseSingleton.h"
#include "Window/SDL2Window/SDL2Window.h"
#include "System/Util/SquirrelFileSystemHelper.h"

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

        if(SQ_FAILED(SquirrelFileSystemHelper::sqstd_dofile(vm, outString.c_str(), false, true))){
            return sq_throwerror(vm, "Error executing script file.");
        }

        return 0;
    }

    SQInteger MiscFunctions::doFileWriteClosure(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, 2, &filePath);

        const SQChar *outFilePath;
        sq_getstring(vm, 3, &outFilePath);

        std::string targetFileString;
        formatResToPath(filePath, targetFileString);

        std::string outFileString;
        formatResToPath(outFilePath, outFileString);

        if(!fileExists(targetFileString)){
            std::string s("Script at path does not exist: ");
            s += targetFileString;
            return sq_throwerror(vm, s.c_str());
        }

        SQObject context;
        sq_getstackobj(vm, -1, &context);

        sq_enabledebuginfo(vm, SQFalse);
        if(SQ_FAILED(sqstd_loadfile(vm, targetFileString.c_str(), SQTrue))){
            sq_enabledebuginfo(vm, SQTrue);
            return sq_throwerror(vm, "Error calling main closure.");
        }
        sq_enabledebuginfo(vm, SQTrue);

        sqstd_writeclosuretofile(vm, outFileString.c_str());

        return 0;
    }

    SQInteger MiscFunctions::doFileWithContext(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, 2, &filePath);
        std::string outString;
        formatResToPath(filePath, outString);

        if(!fileExists(outString)){
            std::string s("Script at path does not exist: ");
            s += outString;
            return sq_throwerror(vm, s.c_str());
        }

        SQObject context;
        sq_getstackobj(vm, -1, &context);

        if(SQ_FAILED(sqstd_loadfile(vm, outString.c_str(), SQTrue))){
            return sq_throwerror(vm, "Error calling main closure.");
        }

        sq_pushobject(vm, context);

        if(SQ_FAILED(sq_call(vm, 1, false, true))){
            //AV_ERROR("Failed to call the main closure in the callback script {}", mFilePath);
            return false;
        }

        sq_pop(vm, 1);

        return 0;
    }

    SQInteger MiscFunctions::compileBuffer(HSQUIRRELVM vm){
        const SQChar *bufferPtr;
        sq_getstring(vm, 2, &bufferPtr);

        size_t len = strlen(bufferPtr);

        if(SQ_FAILED(sq_compilebuffer(vm, bufferPtr, len, "compileBuffer", SQFalse))){
            return sq_throwerror(vm, "Error compiling buffer");
        }

        return 1;
    }

    SQInteger MiscFunctions::getTime(HSQUIRRELVM vm){
        SQInteger t = (SQInteger)time(NULL);
        sq_pushinteger(vm, t);
        return 1;
    }

    SQInteger MiscFunctions::prettyPrint(HSQUIRRELVM vm){
        std::string outString;
        ScriptUtils::_getStringForType(vm, outString);

        sq_pushstring(vm, outString.c_str(), outString.length());

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
        ScriptUtils::addFunction(vm, doFileWithContext, "_doFileWithContext", 3, ".st");
        ScriptUtils::addFunction(vm, doFileWriteClosure, "_doFileWriteClosure", 3, ".ss");
        ScriptUtils::addFunction(vm, compileBuffer, "_compileBuffer", 2, ".s");
        ScriptUtils::addFunction(vm, getTime, "_time");
        ScriptUtils::addFunction(vm, prettyPrint, "_prettyPrint");
        ScriptUtils::addFunction(vm, shutdownEngine, "_shutdownEngine");
    }
}
