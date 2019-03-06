#include "ScriptManager.h"
#include "Logger/Log.h"
#include "ScriptNamespace/CameraNamespace.h"
#include "ScriptNamespace/MeshNamespace.h"
#include "ScriptNamespace/WorldNamespace.h"
#include "ScriptNamespace/SlotManagerNamespace.h"
#include "ScriptNamespace/TestNamespace/TestNamespace.h"
#include "ScriptNamespace/Classes/Vector3Class.h"
#include "ScriptNamespace/Classes/SlotPositionClass.h"

#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"
#include "System/SystemSetup/SystemSettings.h"

#include <iostream>

#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>

#ifdef SQUNICODE
#define scvprintf vwprintf
#else
#define scvprintf vprintf
#endif

namespace AV {
    HSQUIRRELVM ScriptManager::_sqvm = sq_open(1024);
    bool ScriptManager::closed = false;

    void printfunc(HSQUIRRELVM v, const SQChar *s, ...){
        va_list arglist;
        va_start(arglist, s);
        scvprintf(s, arglist);
        va_end(arglist);
        std::cout << '\n';
    }

    void ScriptManager::initialise(){
        _setupVM(_sqvm);
    }

    void ScriptManager::shutdown(){
        if(closed) return;

        sq_close(_sqvm);
        closed = true;
        AV_INFO("Shutdown Squirrel vm.");
    }

    void ScriptManager::runScript(const std::string &scriptPath){
        AV_INFO("Running Script {}", scriptPath);

        int top = sq_gettop(_sqvm);
        sq_pushroottable(_sqvm);
        if(SQ_SUCCEEDED(sqstd_dofile(_sqvm, _SC(scriptPath.c_str()), 0, 1))){
            AV_INFO("Succeeded");
        }else{
            _processSquirrelFailure(scriptPath);
        }
        sq_settop(_sqvm, top);
    }

    void ScriptManager::callFunction(const std::string &scriptPath, const std::string &functionName){
        int top = sq_gettop(_sqvm);
        sq_pushroottable(_sqvm);
        if(SQ_SUCCEEDED(sqstd_loadfile(_sqvm, _SC(scriptPath.c_str()), 1))){

            sq_pushroottable(_sqvm);
            sq_pushstring(_sqvm, _SC(functionName.c_str()), -1);

            if(SQ_SUCCEEDED(sq_get(_sqvm,-2))) {
                sq_pushroottable(_sqvm);

                //number of arguments, no return value, whether to envoke the error handler
                if(!SQ_SUCCEEDED(sq_call(_sqvm, 1, 0, 0))){
                    _processSquirrelFailure(scriptPath);
                }
            }else{
                //There was some problem finding that function in the file.
            }


        }else{
            AV_ERROR("There was a problem running that function.");
        }
        sq_settop(_sqvm, top);
    }

    void ScriptManager::_processSquirrelFailure(const std::string& scriptPath){

        const SQChar* sqErr;
        sq_getlasterror(_sqvm);
        sq_tostring(_sqvm, -1);
        sq_getstring(_sqvm, -1, &sqErr);
        sq_pop(_sqvm, 1);


        if(SystemSettings::isTestModeEnabled()){
            TestingEventScriptFailure event;
            event.srcFile = scriptPath;
            event.failureReason = sqErr;

            EventDispatcher::transmitEvent(EventType::Testing, event);
        }else{
            AV_ERROR("There was a problem running that script file.");
            AV_ERROR(sqErr);
        }
    }

    void ScriptManager::_debugStack(HSQUIRRELVM sq){
        int top = sq_gettop(sq);
        if(top <= 0){
          std::cout << "Nothing in the stack!" << '\n';
          return;
        }
        //This push root table sometimes causes problems.
        //sq_pushroottable(sq);
        while(top >= 0) {
            SQObjectType objectType = sq_gettype(sq, top);
            //Type type = Type(objectType);
            std::cout << "stack index: " << top << " type: " << typeToStr(objectType) << std::endl;
            top--;
        }
    }


    void ScriptManager::injectPointers(Ogre::Camera *camera, Ogre::SceneManager* sceneManager){
        CameraNamespace::_camera = camera;
        MeshNamespace::_sceneManager = sceneManager;
    }

    void ScriptManager::_setupVM(HSQUIRRELVM vm){
        sq_setprintfunc(vm, printfunc, NULL);

        sq_pushroottable(vm);

        sqstd_register_mathlib(vm);
        sqstd_register_systemlib(vm);

        CameraNamespace cameraNamespace;
        MeshNamespace meshNamespace;
        WorldNamespace worldNamespace;
        SlotManagerNamespace slotManagerNamespace;
        TestNamespace testNamespace;
        _createCameraNamespace(vm, cameraNamespace);
        _createMeshNamespace(vm, meshNamespace);
        _createWorldNamespace(vm, worldNamespace);
        _createSlotManagerNamespace(vm, slotManagerNamespace);
        _createTestNamespace(vm, testNamespace);
        _createVec3Class(vm);
        _createSlotPositionClass(vm);

        sq_pop(vm,1);
    }

    void ScriptManager::_createCameraNamespace(HSQUIRRELVM vm, CameraNamespace &cameraNamespace){
        sq_pushstring(vm, _SC("_camera"), -1);
        sq_newtable(vm);

        cameraNamespace.setupNamespace(vm);

        sq_newslot(vm, -3 , false);

    }

    void ScriptManager::_createMeshNamespace(HSQUIRRELVM vm, MeshNamespace &meshNamespace){
        sq_pushstring(vm, _SC("_mesh"), -1);
        sq_newtable(vm);

        meshNamespace.setupNamespace(vm);

        sq_newslot(vm, -3 , false);
    }

    void ScriptManager::_createWorldNamespace(HSQUIRRELVM vm, WorldNamespace &worldNamespace){
        sq_pushstring(vm, _SC("_world"), -1);
        sq_newtable(vm);

        worldNamespace.setupNamespace(vm);

        sq_newslot(vm, -3 , false);
    }

    void ScriptManager::_createSlotManagerNamespace(HSQUIRRELVM vm, SlotManagerNamespace &slotManagerNamespace){
        sq_pushstring(vm, _SC("_slotManager"), -1);
        sq_newtable(vm);

        slotManagerNamespace.setupNamespace(vm);

        sq_newslot(vm, -3 , false);
    }

    void ScriptManager::_createTestNamespace(HSQUIRRELVM vm, TestNamespace &testNamespace){
        sq_pushstring(vm, _SC("_test"), -1);
        sq_newtable(vm);

        testNamespace.setupNamespace(vm);

        sq_newslot(vm, -3 , false);
    }

    void ScriptManager::_createVec3Class(HSQUIRRELVM vm){
        Vector3Class vec;
        vec.setupClass(vm);
    }

    void ScriptManager::_createSlotPositionClass(HSQUIRRELVM vm){
        SlotPositionClass slot;
        slot.setupClass(vm);
    }
}
