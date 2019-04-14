#include "ScriptManager.h"
#include "Logger/Log.h"
#include "ScriptNamespace/ScriptNamespace.h"
#include "ScriptNamespace/CameraNamespace.h"
#include "ScriptNamespace/MeshNamespace.h"
#include "ScriptNamespace/WorldNamespace.h"
#include "ScriptNamespace/SlotManagerNamespace.h"
#include "ScriptNamespace/EntityNamespace.h"
#include "ScriptNamespace/TestNamespace/TestNamespace.h"
#include "ScriptNamespace/ComponentNamespace.h"
#include "ScriptNamespace/InputNamespace.h"
#include "ScriptNamespace/ScriptingStateNamespace.h"
#include "ScriptNamespace/Classes/Vector3Class.h"
#include "ScriptNamespace/Classes/SlotPositionClass.h"
#include "ScriptNamespace/Classes/EntityClass/EntityClass.h"

#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"
#include "System/SystemSetup/SystemSettings.h"

#include <iostream>

#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>

#include "Script/Script.h"
#include "Script/CallbackScript.h"

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

    void ScriptManager::initialiseScript(Script* s){
        s->initialise(_sqvm);
    }

    void ScriptManager::initialiseCallbackScript(CallbackScript *s){
        s->initialise(_sqvm);
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

    void ScriptManager::injectPointers(Ogre::Camera *camera, Ogre::SceneManager* sceneManager, ScriptingStateManager* stateManager){
        CameraNamespace::_camera = camera;
        MeshNamespace::_sceneManager = sceneManager;
        ScriptingStateNamespace::stateManager = stateManager;
    }

    void ScriptManager::_setupVM(HSQUIRRELVM vm){
        sq_setprintfunc(vm, printfunc, NULL);

        //Setup the root table.
        sq_pushroottable(vm);

        sqstd_register_mathlib(vm);
        sqstd_register_systemlib(vm);
        sqstd_register_iolib(vm);

        CameraNamespace cameraNamespace;
        MeshNamespace meshNamespace;
        WorldNamespace worldNamespace;
        SlotManagerNamespace slotManagerNamespace;
        TestNamespace testNamespace;
        EntityNamespace entityNamespace;
        ComponentNamespace componentNamespace;
        ScriptingStateNamespace scriptingState;
        InputNamespace inputNamespace;

        const int namespaceEntries = 9;
        ScriptNamespace* n[namespaceEntries] = {
            &cameraNamespace,
            &meshNamespace,
            &worldNamespace,
            &slotManagerNamespace,
            &testNamespace,
            &entityNamespace,
            &componentNamespace,
            &scriptingState,
            &inputNamespace
        };
        const char* names[namespaceEntries] = {
            "_camera",
            "_mesh",
            "_world",
            "_slotManager",
            "_test",
            "_entity",
            "_component",
            "_scriptingState",
            "_input"
        };
        for(int i = 0; i < namespaceEntries; i++){
            _createNamespace(vm, n[i], names[i]);
        }

        _createVec3Class(vm);
        _createSlotPositionClass(vm);
        _setupEntityClass(vm);
        
        inputNamespace.setupConstants(vm);

        sq_pop(vm,1);
    }

    void ScriptManager::_createNamespace(HSQUIRRELVM vm, ScriptNamespace *n, const char* namespaceName){
        sq_pushstring(vm, _SC(namespaceName), -1);
        sq_newtable(vm);

        n->setupNamespace(vm);

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

    void ScriptManager::_setupEntityClass(HSQUIRRELVM vm){
        EntityClass eClass;
        eClass.setupClass(vm);
    }
}
