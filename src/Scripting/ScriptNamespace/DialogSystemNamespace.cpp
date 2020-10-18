#include "DialogSystemNamespace.h"

#include "System/BaseSingleton.h"
#include "Dialog/DialogManager.h"

#include "Dialog/Compiler/DialogScriptData.h"
#include "Dialog/Compiler/DialogCompiler.h"
#include "System/Util/PathUtils.h"

#include "GlobalRegistryNamespace.h"

namespace AV{
    SQInteger DialogSystemNamespace::unblock(HSQUIRRELVM vm){
        BaseSingleton::getDialogManager()->unblock();

        return 0;
    }

    SQInteger DialogSystemNamespace::dialogReleaseHook(SQUserPointer p, SQInteger size){
        CompiledDialog* dialogPtr = static_cast<CompiledDialog*>(p);

        BaseSingleton::getDialogManager()->notifyDialogDeletion(dialogPtr);
        dialogPtr->destroy();

        return 0;
    }

    SQInteger DialogSystemNamespace::compileAndRunDialog(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, -1, &path);

        std::string outString;
        formatResToPath(path, outString);

        bool result = BaseSingleton::getDialogManager()->compileAndRunDialog(outString);

        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger DialogSystemNamespace::compileDialog(HSQUIRRELVM vm){
        const SQChar *dialogPath;
        sq_getstring(vm, -1, &dialogPath);

        std::string outString;
        formatResToPath(dialogPath, outString);
        sq_pop(vm, 1);

        CompiledDialog d;
        DialogCompiler compiler;
        if(!compiler.compileScript(outString, d)){
            return sq_throwerror(vm, "Error compiling dialog script.");
        }


        void* pointer = (void*)sq_newuserdata(vm, sizeof(CompiledDialog));
        new (pointer)CompiledDialog();
        CompiledDialog* dialogPtr = static_cast<CompiledDialog*>(pointer);
        *dialogPtr = d;

        sq_setreleasehook(vm, -1, dialogReleaseHook);

        return 1;
    }

    SQInteger DialogSystemNamespace::executeCompiledDialog(HSQUIRRELVM vm){
        SQInteger nargs = sq_gettop(vm);
        SQInteger targetBlock = 0;
        if(nargs == 3){
            //The user specified a block to start on.
            sq_getinteger(vm, -1, &targetBlock);
            sq_pop(vm, 1);
        }

        SQUserPointer pointer;
        sq_getuserdata(vm, -1, &pointer, NULL);
        if(!pointer) return sq_throwerror(vm, "Unable to read data from compiled dialog.");
        CompiledDialog* dialogPtr = static_cast<CompiledDialog*>(pointer);

        BaseSingleton::getDialogManager()->beginExecution(*dialogPtr, targetBlock);

        return 0;
    }

    SQInteger DialogSystemNamespace::isDialogExecuting(HSQUIRRELVM vm){
        SQBool result = BaseSingleton::getDialogManager()->isExecuting();
        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger DialogSystemNamespace::isDialogBlocked(HSQUIRRELVM vm){
        SQBool result = BaseSingleton::getDialogManager()->isBlocked();
        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger DialogSystemNamespace::updateDialogSystem(HSQUIRRELVM vm){
        BaseSingleton::getDialogManager()->update();

        return 0;
    }


    SQInteger setValue(HSQUIRRELVM vm) { return GlobalRegistryNamespace::setValue(vm, false); }
    SQInteger getValue(HSQUIRRELVM vm) { return GlobalRegistryNamespace::getValue(vm, false); }

    SQInteger getInt(HSQUIRRELVM vm) { return GlobalRegistryNamespace::getInt(vm, false); }
    SQInteger getBool(HSQUIRRELVM vm) { return GlobalRegistryNamespace::getBool(vm, false); }
    SQInteger getFloat(HSQUIRRELVM vm) { return GlobalRegistryNamespace::getFloat(vm, false); }
    SQInteger getString(HSQUIRRELVM vm) { return GlobalRegistryNamespace::getString(vm, false); }

    SQInteger clear(HSQUIRRELVM vm) { return GlobalRegistryNamespace::clear(vm, false); }


    /**SQNamespace
    @name _dialog
    @desc Squirrel functions to interact with the dialog system.
    */
    void DialogSystemNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name unblock
        @desc Unblock the execution of the dialog.
        */
        ScriptUtils::addFunction(vm, unblock, "unblock");
        /**SQFunction
        @name compileAndRunDialog
        @param1:path: A res path to a dialog script file.
        @desc Compile a dialog script and begin execution. This does not create any sort of reusable reference to the compiled dialog. It's more there for convenience.
        */
        ScriptUtils::addFunction(vm, compileAndRunDialog, "compileAndRunDialog", 2, ".s");
        /**SQFunction
        @name compileDialog
        @param1:path: A res path to a dilog script file.
        @desc Compile a dialog script and return it. This function does not execute the dialog.
        @returns A compiled dialog script.
        */
        ScriptUtils::addFunction(vm, compileDialog, "compileDialog", 2, ".s");
        /**SQFunction
        @name executeCompiledDialog
        @param1:compiledDialog: A dialog compiled by the compileDialog function.
        @param2:blockId: An integer representing a block to begin execution with. Defaults to 0.
        @desc Begin execution of a compiled dialog.
        */
        ScriptUtils::addFunction(vm, executeCompiledDialog, "executeCompiledDialog", -2, ".di");
        /**SQFunction
        @name update
        @desc Perform an update of the dialog system. Update is called by the engine each frame. This function is really just a utility function, useful for testing.
        */
        ScriptUtils::addFunction(vm, updateDialogSystem, "update");
        /**SQFunction
        @name isDialogExecuting
        @returns True or false depending on whether the dialog is currently executing.
        */
        ScriptUtils::addFunction(vm, isDialogExecuting, "isDialogExecuting");
        /**SQFunction
        @name isDialogExecuting
        @returns True or false depending on whether the dialog is currently blocked.
        */
        ScriptUtils::addFunction(vm, isDialogBlocked, "isDialogBlocked");

        {
            sq_pushstring(vm, _SC("registry"), -1);
            sq_newtable(vm);

            ScriptUtils::addFunction(vm, getInt, "getInt", 2, ".s");
            ScriptUtils::addFunction(vm, getFloat, "getFloat", 2, ".s");
            ScriptUtils::addFunction(vm, getBool, "getBool", 2, ".s");
            ScriptUtils::addFunction(vm, getString, "getString", 2, ".s");
            ScriptUtils::addFunction(vm, getValue, "get", 2, ".s");
            ScriptUtils::addFunction(vm, setValue, "set", 3, ".s.");
            ScriptUtils::addFunction(vm, clear, "clear");

            sq_newslot(vm, -3 , false);
        }
    }

}
