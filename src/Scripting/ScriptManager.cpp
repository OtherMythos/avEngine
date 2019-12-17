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
#include "ScriptNamespace/SettingsNamespace.h"
#include "ScriptNamespace/SerialisationNamespace.h"
#include "ScriptNamespace/PhysicsNamespace.h"
#include "ScriptNamespace/WindowNamespace.h"

#include "ScriptNamespace/Classes/Vector3Class.h"
#include "ScriptNamespace/Classes/SlotPositionClass.h"
#include "ScriptNamespace/Classes/SaveHandleClass.h"
#include "ScriptNamespace/Classes/EntityClass/EntityClass.h"
#include "ScriptNamespace/Classes/PhysicsClasses/PhysicsShapeClass.h"
#include "ScriptNamespace/Classes/PhysicsClasses/PhysicsRigidBodyClass.h"
#include "ScriptNamespace/Classes/MeshClass.h"
#include "ScriptNamespace/Classes/MovableTextureClass.h"

#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"
#include "System/SystemSetup/SystemSettings.h"

#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>

#include "Script/Script.h"
#include "Script/CallbackScript.h"

#include "Debugger/ScriptDebugger.h"

#ifdef SQUNICODE
#define scvprintf vwprintf
#else
#define scvprintf vprintf
#endif

namespace AV {
    HSQUIRRELVM ScriptManager::_sqvm;
    bool ScriptManager::closed = false;
    ScriptDebugger* ScriptManager::mDebugger = 0;

    #ifdef TEST_MODE
        bool ScriptManager::testFinished = false;
    #endif

    void printfunc(HSQUIRRELVM v, const SQChar *s, ...){
        va_list arglist;
        va_start(arglist, s);
        scvprintf(s, arglist);
        va_end(arglist);
        std::cout << '\n';
    }

    static SQInteger errorHandler(HSQUIRRELVM vm){
        #ifdef TEST_MODE
            if(ScriptManager::hasTestFinished()) return 0;
        #endif

        const SQChar* sqErr;
        sq_getlasterror(vm);
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        sq_pop(vm, 1);


        SQStackInfos si;
        sq_stackinfos(vm, 1, &si);

        static const std::string separator(10, '=');

        AV_ERROR(separator);

        AV_ERROR("Error during script execution.");
        AV_ERROR(sqErr);
        AV_ERROR("In file {}", si.source);
        AV_ERROR("    on line {}", si.line);
        AV_ERROR("of function {}", si.funcname);

        AV_ERROR(separator);

        #ifdef TEST_MODE
        if(SystemSettings::isTestModeEnabled()){
            //If any scripts fail during a test mode run, the engine is shut down and the test is failed.
            TestingEventScriptFailure event;
            event.srcFile = si.source;
            event.failureReason = sqErr;
            //event.functionName = si.funcname;
            //event.lineNum = si.line;

            EventDispatcher::transmitEvent(EventType::Testing, event);
        }
        #endif

        return 0;
    }

    static void compilerError(HSQUIRRELVM vm, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column){
        #ifdef TEST_MODE
            if(ScriptManager::hasTestFinished()) return;
        #endif

        static const std::string separator(10, '=');

        AV_ERROR(separator);

        AV_ERROR("Error during script compilation.");
        AV_ERROR(desc);
        AV_ERROR("In file {}", source);
        AV_ERROR("    on line {} column {}", line, column);

        AV_ERROR(separator);

        #ifdef TEST_MODE
        if(SystemSettings::isTestModeEnabled()){
            //If any scripts fail during a test mode run, the engine is shut down and the test is failed.
            TestingEventScriptFailure event;
            event.srcFile = source;
            event.failureReason = desc;
            //event.functionName = si.funcname;
            //event.lineNum = si.line;

            EventDispatcher::transmitEvent(EventType::Testing, event);
        }
        #endif
    }

    void ScriptManager::_initialiseVM(){
        _sqvm = sq_open(1024);

        sq_setprintfunc(_sqvm, printfunc, NULL);

        sq_enabledebuginfo(_sqvm, true);

        sq_newclosure(_sqvm, errorHandler, 0);
        sq_seterrorhandler(_sqvm);

        sq_setcompilererrorhandler(_sqvm, compilerError);
    }

    void ScriptManager::initialise(){
        #ifdef TEST_MODE
            EventDispatcher::subscribeStatic(EventType::Testing, AV_BIND_STATIC(ScriptManager::testEventReceiver));
        #endif

        _initialiseVM();
        _setupVM(_sqvm);

        mDebugger = new ScriptDebugger(_sqvm);
    }

    void ScriptManager::shutdown(){
        if(closed) return;

        delete mDebugger;

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

    #ifdef TEST_MODE
    bool ScriptManager::testEventReceiver(const Event &e){
        const TestingEvent& testEvent = (TestingEvent&)e;
        if(testEvent.eventCategory() == TestingEventCategory::testEnd){
            testFinished = true;
        }
    }
    #endif

    void ScriptManager::injectPointers(Ogre::Camera *camera, Ogre::SceneManager* sceneManager, ScriptingStateManager* stateManager){
        CameraNamespace::_camera = camera;
        ScriptingStateNamespace::stateManager = stateManager;
    }

    void ScriptManager::_setupVM(HSQUIRRELVM vm){
        //Setup the root table.
        sq_pushroottable(vm);

        sqstd_register_mathlib(vm);
        sqstd_register_systemlib(vm);
        sqstd_register_iolib(vm);

        CameraNamespace cameraNamespace;
        MeshNamespace meshNamespace;
        WorldNamespace worldNamespace;
        SlotManagerNamespace slotManagerNamespace;
        #ifdef TEST_MODE
            TestNamespace testNamespace;
        #endif
        EntityNamespace entityNamespace;
        ComponentNamespace componentNamespace;
        ScriptingStateNamespace scriptingState;
        InputNamespace inputNamespace;
        SettingsNamespace settingsNamespace;
        SerialisationNamespace serialisationNamespace;
        PhysicsNamespace physicsNamespace;
        WindowNamespace windowNamespace;

        #ifdef TEST_MODE
            const int namespaceEntries = 13;
        #else
            const int namespaceEntries = 12;
        #endif
        ScriptNamespace* n[namespaceEntries] = {
            &cameraNamespace,
            &meshNamespace,
            &worldNamespace,
            &slotManagerNamespace,
            #ifdef TEST_MODE
                &testNamespace,
            #endif
            &entityNamespace,
            &componentNamespace,
            &scriptingState,
            &inputNamespace,
            &settingsNamespace,
            &serialisationNamespace,
            &physicsNamespace,
            &windowNamespace
        };
        const char* names[namespaceEntries] = {
            "_camera",
            "_mesh",
            "_world",
            "_slotManager",
            #ifdef TEST_MODE
                "_test",
            #endif
            "_entity",
            "_component",
            "_scriptingState",
            "_input",
            "_settings",
            "_serialisation",
            "_physics",
            "_window"
        };
        for(int i = 0; i < namespaceEntries; i++){
            _createNamespace(vm, n[i], names[i]);
        }

        _createVec3Class(vm);
        _createSlotPositionClass(vm);
        _setupEntityClass(vm);
        _setupSaveHandleClass(vm);
        _setupPhysicsShapeClass(vm);
        _setupPhysicsRigidBodyClass(vm);
        _setupMeshClass(vm);
        _setupMovableTextureClass(vm);

        inputNamespace.setupConstants(vm);
        settingsNamespace.setupConstants(vm);

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

    void ScriptManager::_setupSaveHandleClass(HSQUIRRELVM vm){
        SaveHandleClass sClass;
        sClass.setupClass(vm);
    }

    void ScriptManager::_setupPhysicsShapeClass(HSQUIRRELVM vm){
        PhysicsShapeClass c;
        c.setupClass(vm);
    }

    void ScriptManager::_setupPhysicsRigidBodyClass(HSQUIRRELVM vm){
        PhysicsRigidBodyClass c;
        c.setupClass(vm);
    }

    void ScriptManager::_setupMeshClass(HSQUIRRELVM vm){
        MeshClass c;
        c.setupClass(vm);
    }

    void ScriptManager::_setupMovableTextureClass(HSQUIRRELVM vm){
        MovableTextureClass c;
        c.setupClass(vm);
    }
}
