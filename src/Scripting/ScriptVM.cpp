#include "ScriptVM.h"
#include "Logger/Log.h"
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
#include "ScriptNamespace/DialogSystemNamespace.h"
#include "ScriptNamespace/HlmsNamespace.h"
#include "ScriptNamespace/GlobalRegistryNamespace.h"
#include "ScriptNamespace/RandomNamespace.h"
#include "ScriptNamespace/GuiNamespace.h"
#include "ScriptNamespace/DeveloperNamespace.h"

#include "ScriptNamespace/MiscFunctions.h"

#include "ScriptNamespace/Classes/SlotPositionClass.h"
#include "ScriptNamespace/Classes/SaveHandleClass.h"
#include "ScriptNamespace/Classes/EntityClass/EntityClass.h"
#include "ScriptNamespace/Classes/PhysicsClasses/PhysicsShapeClass.h"
#include "ScriptNamespace/Classes/PhysicsClasses/PhysicsRigidBodyClass.h"
#include "ScriptNamespace/Classes/PhysicsClasses/PhysicsSenderClass.h"
#include "ScriptNamespace/Classes/MeshClass.h"
#include "ScriptNamespace/Classes/MovableTextureClass.h"
#include "ScriptNamespace/Classes/Rect2dClass.h"
#include "ScriptNamespace/Classes/FileClass.h"

#include "ScriptNamespace/Classes/Ogre/DatablockUserData.h"
#include "ScriptNamespace/Classes/Vector3UserData.h"

#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"
#include "System/SystemSetup/SystemSettings.h"

#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>
#include <sqstdstring.h>

#include "Script/Script.h"
#include "Script/CallbackScript.h"

#ifdef DEBUGGING_TOOLS
    #include "Debugger/ScriptDebugger.h"
#endif

#ifdef SQUNICODE
#define scvprintf vwprintf
#else
#define scvprintf vprintf
#endif

namespace AV {
    HSQUIRRELVM ScriptVM::_sqvm;
    bool ScriptVM::closed = false;

    #ifdef DEBUGGING_TOOLS
        ScriptDebugger* ScriptVM::mDebugger = 0;
    #endif
    #ifdef TEST_MODE
        bool ScriptVM::testFinished = false;
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
            if(ScriptVM::hasTestFinished()) return 0;
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
            if(ScriptVM::hasTestFinished()) return;
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

    void ScriptVM::_initialiseVM(){
        _sqvm = sq_open(1024);

        sq_setprintfunc(_sqvm, printfunc, NULL);

        sq_enabledebuginfo(_sqvm, true);

        sq_newclosure(_sqvm, errorHandler, 0);
        sq_seterrorhandler(_sqvm);

        sq_setcompilererrorhandler(_sqvm, compilerError);
    }

    void ScriptVM::initialise(){
        #ifdef TEST_MODE
            EventDispatcher::subscribeStatic(EventType::Testing, AV_BIND_STATIC(ScriptVM::testEventReceiver));
        #endif

        _initialiseVM();
        _setupVM(_sqvm);

        #ifdef DEBUGGING_TOOLS
            mDebugger = new ScriptDebugger(_sqvm);
        #endif
    }

    void ScriptVM::shutdown(){
        if(closed) return;

        #ifdef DEBUGGING_TOOLS
            delete mDebugger;
        #endif

        sq_close(_sqvm);
        closed = true;
        AV_INFO("Shutdown Squirrel vm.");
    }

    void ScriptVM::initialiseScript(Script* s){
        s->initialise(_sqvm);
    }

    void ScriptVM::initialiseCallbackScript(CallbackScript *s){
        s->initialise(_sqvm);
    }

    #ifdef TEST_MODE
    bool ScriptVM::testEventReceiver(const Event &e){
        const TestingEvent& testEvent = (TestingEvent&)e;
        if(testEvent.eventCategory() == TestingEventCategory::testEnd){
            testFinished = true;
        }
        return false;
    }
    #endif

    void ScriptVM::injectPointers(Ogre::Camera *camera, Ogre::SceneManager* sceneManager, ScriptingStateManager* stateManager){
        CameraNamespace::_camera = camera;
        ScriptingStateNamespace::stateManager = stateManager;
    }

    void ScriptVM::_setupVM(HSQUIRRELVM vm){
        //Setup the root table.
        sq_pushroottable(vm);

        sqstd_register_mathlib(vm);
        //sqstd_register_systemlib(vm);
        //sqstd_register_iolib(vm);
        sqstd_register_stringlib(vm);

        typedef void(*SetupFunction)(HSQUIRRELVM vm);
        typedef std::pair<const char*, SetupFunction> NamespaceEntry;

        const std::vector<NamespaceEntry> namespaces = {
            {"_camera", CameraNamespace::setupNamespace},
            {"_mesh", MeshNamespace::setupNamespace},
            {"_world", WorldNamespace::setupNamespace},
            {"_slotManager", SlotManagerNamespace::setupNamespace},
            {"_entity", EntityNamespace::setupNamespace},
            {"_component", ComponentNamespace::setupNamespace},
            {"_scriptingState", ScriptingStateNamespace::setupNamespace},
            {"_input", InputNamespace::setupNamespace},
            {"_settings", SettingsNamespace::setupNamespace},
            {"_serialisation", SerialisationNamespace::setupNamespace},
            {"_physics", PhysicsNamespace::setupNamespace},
            {"_window", WindowNamespace::setupNamespace},
            {"_dialogSystem", DialogSystemNamespace::setupNamespace},
            #ifdef TEST_MODE
                {"_test", TestNamespace::setupNamespace},
            #endif
            {"_hlms", HlmsNamespace::setupNamespace},
            {"_registry", GlobalRegistryNamespace::setupNamespace},
            {"_random", RandomNamespace::setupNamespace},
            {"_gui", GuiNamespace::setupNamespace},
            #ifdef DEBUGGING_TOOLS
                {"_developer", DeveloperNamespace::setupNamespace},
            #endif
        };

        for(const NamespaceEntry& e : namespaces){
            sq_pushstring(vm, _SC(e.first), -1);
            sq_newtable(vm);

            e.second(vm);

            sq_newslot(vm, -3 , false);
        }

        MiscFunctions::setupFunctions(vm);

        SlotPositionClass::setupClass(vm);
        EntityClass::setupClass(vm);
        SaveHandleClass::setupClass(vm);
        PhysicsShapeClass::setupClass(vm);
        PhysicsRigidBodyClass::setupClass(vm);
        MeshClass::setupClass(vm);
        MovableTextureClass::setupClass(vm);
        Rect2dClass::setupClass(vm);
        FileClass::setupClass(vm);
        PhysicsSenderClass::setupClass(vm);

        DatablockUserData::setupDelegateTable(vm);
        Vector3UserData::setupTable(vm);

        InputNamespace::setupConstants(vm);
        SettingsNamespace::setupConstants(vm);

        sq_pop(vm,1); //Pop the root table.
    }
}
