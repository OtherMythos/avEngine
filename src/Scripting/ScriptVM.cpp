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
#include "ScriptNamespace/SceneNamespace.h"
#include "ScriptNamespace/EventNamespace.h"
#include "ScriptNamespace/NavMeshNamespace.h"
#include "ScriptNamespace/TimerNamespace.h"
#include "ScriptNamespace/AnimationNamespace.h"
#include "ScriptNamespace/StateNamespace.h"
#include "ScriptNamespace/CompositorNamespace.h"
#include "ScriptNamespace/GraphicsNamespace.h"
#include "ScriptNamespace/SystemNamespace.h"
#include "ScriptNamespace/ResourcesNamespace.h"
#include "ScriptNamespace/AudioNamespace.h"
#include "ScriptNamespace/SystemNamespace.h"

#include "ScriptNamespace/MiscFunctions.h"

#include "ScriptNamespace/Classes/Audio/AudioSourceUserData.h"
#include "ScriptNamespace/Classes/Audio/AudioBufferUserData.h"

#include "ScriptNamespace/Classes/SlotPositionClass.h"
#include "ScriptNamespace/Classes/SaveHandleClass.h"
#include "ScriptNamespace/Classes/Entity/EntityUserData.h"
#include "ScriptNamespace/Classes/PhysicsClasses/PhysicsShapeClass.h"
#include "ScriptNamespace/Classes/PhysicsClasses/PhysicsRigidBodyClass.h"
#include "ScriptNamespace/Classes/PhysicsClasses/PhysicsObjectUserData.h"
#include "ScriptNamespace/Classes/MeshClass.h"
#include "ScriptNamespace/Classes/MovableTextureClass.h"
#include "ScriptNamespace/Classes/Rect2dClass.h"
#include "ScriptNamespace/Classes/FileClass.h"
#include "Scripting/ScriptNamespace/Classes/Gui/GuiSizerDelegate.h"

#include "ScriptNamespace/Classes/Ogre/Hlms/DatablockUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/SceneNodeUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/MovableObjectUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/Skeleton/SkeletonUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/Skeleton/SkeletonAnimationUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/Skeleton/BoneUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/AabbUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/RayUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/StagingTextureUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/TextureBoxUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/GPUProgramUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/GPUProgramParametersUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/MaterialUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/MeshUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/SubMeshUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/VertexArrayObjectUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/OgreBufferUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/VertexElementVecUserData.h"
#include "ScriptNamespace/Classes/Ogre/Compositor/CompositorWorkspaceUserData.h"
#include "ScriptNamespace/Classes/Nav/NavMeshUserData.h"
#include "ScriptNamespace/Classes/Nav/NavMeshQueryUserData.h"
#include "ScriptNamespace/Classes/Animation/AnimationInstanceUserData.h"
#include "ScriptNamespace/Classes/TimerUserData.h"

#include "ScriptNamespace/Classes/Vector3UserData.h"
#include "ScriptNamespace/Classes/Vector2UserData.h"
#include "ScriptNamespace/Classes/ColourValueUserData.h"
#include "ScriptNamespace/Classes/QuaternionUserData.h"
#include "ScriptNamespace/Classes/PlaneUserData.h"

#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"
#include "System/SystemSetup/SystemSettings.h"

#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>
#include <sqstdstring.h>
#include <sqstdblob.h>

#include "Script/Script.h"
#include "Script/CallbackScript.h"

#ifdef DEBUGGING_TOOLS
    #include "Debugger/ScriptDebugger.h"
#endif

#include "Logger/Log.h"
#include <cstdio>

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
        char buffer[256];
        va_list args;
        va_start (args, s);
        vsnprintf (buffer, 256, s, args);
        va_end (args);

        AV_SQUIRREL_PRINT("{}", buffer);
    }

    SQInteger ScriptVM::errorHandler(HSQUIRRELVM vm){
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

            ScriptUtils::_debugBacktrace(_sqvm);

            EventDispatcher::transmitEvent(EventType::Testing, event);
            return 0;
        }
        #endif

        #ifdef DEBUGGING_TOOLS
            mDebugger->pendDebugging();
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

    void ScriptVM::referenceObject(SQObject closure){
        sq_addref(_sqvm, &closure);
    }

    void ScriptVM::dereferenceObject(SQObject closure){
        sq_release(_sqvm, &closure);
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

    bool ScriptVM::callClosure(HSQOBJECT closure, const HSQOBJECT* context, PopulateFunction func){
        sq_pushobject(_sqvm, closure);
        if(context){
            sq_pushobject(_sqvm, *context);
        }else{
            sq_pushroottable(_sqvm);
        }

        SQInteger paramCount = 1;
        if(func){
            paramCount = (func)(_sqvm);
        }

        if(SQ_FAILED(sq_call(_sqvm, paramCount, false, true))){
            return false;
        }
        sq_pop(_sqvm, 1);

        return true;
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
        if(testEvent.eventId() == EventId::TestingTestEnd){
            testFinished = true;
        }
        return false;
    }
    #endif

    void ScriptVM::injectPointers(Ogre::Camera *camera, Ogre::SceneManager* sceneManager, ScriptingStateManager* stateManager){
        CameraNamespace::_camera = camera;
        ScriptingStateNamespace::stateManager = stateManager;
        SceneNamespace::_scene = sceneManager;
        CompositorNamespace::_scene = sceneManager;

        TextureUserData::setupListener();
    }

    void ScriptVM::_setupVM(HSQUIRRELVM vm){
        //Setup the root table.
        sq_pushroottable(vm);

        sqstd_register_mathlib(vm);
        //sqstd_register_systemlib(vm);
        //sqstd_register_iolib(vm);
        sqstd_register_stringlib(vm);
        sqstd_register_bloblib(vm);

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
            {"_scene", SceneNamespace::setupNamespace},
            #ifdef DEBUGGING_TOOLS
                {"_developer", DeveloperNamespace::setupNamespace},
            #endif
            {"_event", EventNamespace::setupNamespace},
            {"_nav", NavMeshNamespace::setupNamespace},
            {"_timer", TimerNamespace::setupNamespace},
            {"_animation", AnimationNamespace::setupNamespace},
            {"_state", StateNamespace::setupNamespace},
            {"_compositor", CompositorNamespace::setupNamespace},
            {"_graphics", GraphicsNamespace::setupNamespace},
            {"_system", SystemNamespace::setupNamespace},
            {"_resources", ResourcesNamespace::setupNamespace},
            {"_audio", AudioNamespace::setupNamespace},
        };

        for(const NamespaceEntry& e : namespaces){
            sq_pushstring(vm, _SC(e.first), -1);
            sq_newtable(vm);

            e.second(vm);

            sq_newslot(vm, -3 , false);
        }

        MiscFunctions::setupFunctions(vm);

        SlotPositionClass::setupClass(vm);
        SaveHandleClass::setupClass(vm);
        PhysicsShapeClass::setupClass(vm);
        PhysicsRigidBodyClass::setupClass(vm);
        MeshClass::setupClass(vm);
        MovableTextureClass::setupClass(vm);
        Rect2dClass::setupClass(vm);
        FileClass::setupClass(vm);

        DatablockUserData::setupDelegateTable(vm);
        Vector3UserData::setupTable(vm);
        Vector2UserData::setupTable(vm);
        QuaternionUserData::setupTable(vm);
        ColourValueUserData::setupTable(vm);
        PhysicsObjectUserData::setupDelegateTable(vm);
        SceneNodeUserData::setupDelegateTable(vm);
        MovableObjectUserData::setupDelegateTable(vm);
        SkeletonUserData::setupDelegateTable(vm);
        SkeletonAnimationUserData::setupDelegateTable(vm);
        BoneUserData::setupDelegateTable(vm);
        AabbUserData::setupDelegateTable(vm);
        RayUserData::setupDelegateTable(vm);
        NavMeshUserData::setupDelegateTable(vm);
        NavMeshQueryUserData::setupDelegateTable(vm);
        AnimationInstanceUserData::setupDelegateTable(vm);
        TextureUserData::setupDelegateTable(vm);
        StagingTextureUserData::setupDelegateTable(vm);
        TextureBoxUserData::setupDelegateTable(vm);
        GPUProgramUserData::setupDelegateTable(vm);
        MaterialUserData::setupDelegateTable(vm);
        MeshUserData::setupDelegateTable(vm);
        SubMeshUserData::setupDelegateTable(vm);
        VertexArrayObjectUserData::setupDelegateTable(vm);
        OgreBufferUserData::setupDelegateTable(vm);
        GPUProgramParametersUserData::setupDelegateTable(vm);
        CompositorWorkspaceUserData::setupDelegateTable(vm);
        EntityUserData::setupDelegateTable(vm);
        AudioSourceUserData::setupDelegateTable(vm);
        AudioBufferUserData::setupDelegateTable(vm);
        PlaneUserData::setupDelegateTable(vm);
        VertexElementVecUserData::setupDelegateTable(vm);
        TimerUserData::setupDelegateTable(vm);

        InputNamespace::setupConstants(vm);
        SettingsNamespace::setupConstants(vm);
        PhysicsNamespace::setupConstants(vm);
        SceneNodeUserData::setupConstants(vm);
        MovableObjectUserData::setupConstants(vm);
        EventNamespace::setupConstants(vm);
        GuiNamespace::setupConstants(vm);
        GuiSizerDelegate::setupConstants(vm);
        StateNamespace::setupConstants(vm);
        TextureUserData::setupConstants(vm);
        HlmsNamespace::setupConstants(vm);
        CameraNamespace::setupConstants(vm);
        WindowNamespace::setupConstants(vm);
        SystemNamespace::setupConstants(vm);
        GPUProgramUserData::setupConstants(vm);
        GraphicsNamespace::setupConstants(vm);
        VertexElementVecUserData::setupConstants(vm);
        SubMeshUserData::setupConstants(vm);

        sq_pop(vm,1); //Pop the root table.
    }
}
