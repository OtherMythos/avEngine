#include "ScriptVM.h"
#include "ScriptVMShared.h"
#include "Logger/Log.h"
#include "ScriptNamespace/CameraNamespace.h"
#include "ScriptNamespace/MeshNamespace.h"
#include "ScriptNamespace/EntityNamespace.h"
#include "ScriptNamespace/TestNamespace/TestNamespace.h"
#include "ScriptNamespace/ComponentNamespace.h"
#include "ScriptNamespace/InputNamespace.h"
#include "ScriptNamespace/ScriptingStateNamespace.h"
#include "ScriptNamespace/SettingsNamespace.h"
#include "ScriptNamespace/PhysicsNamespace.h"
#include "ScriptNamespace/WindowNamespace.h"
#include "ScriptNamespace/HlmsNamespace.h"
#include "ScriptNamespace/GlobalRegistryNamespace.h"
#include "ScriptNamespace/PluginNamespace.h"
#include "ScriptNamespace/DataStoreNamespace.h"
#include "ScriptNamespace/RandomNamespace.h"
#include "ScriptNamespace/GuiNamespace.h"
#include "ScriptNamespace/DeveloperNamespace.h"
#include "ScriptNamespace/SceneNamespace.h"
#include "ScriptNamespace/EventNamespace.h"
#include "ScriptNamespace/AnimationNamespace.h"
#include "ScriptNamespace/StateNamespace.h"
#include "ScriptNamespace/CompositorNamespace.h"
#include "ScriptNamespace/GraphicsNamespace.h"
#include "ScriptNamespace/SystemNamespace.h"
#include "ScriptNamespace/ResourcesNamespace.h"
#include "ScriptNamespace/AudioNamespace.h"
#include "ScriptNamespace/SystemNamespace.h"
#include "ScriptNamespace/LottieNamespace.h"
#include "ScriptNamespace/ScriptWorkerNamespace.h"
#include "ScriptNamespace/Classes/ScriptWorkerHandleUserData.h"
#ifdef ENABLE_MONETISATION
    #include "ScriptNamespace/MonetisationNamespace.h"
#endif

#include "System/Base.h"

#include "ScriptNamespace/MiscFunctions.h"

#include "ScriptNamespace/Classes/Audio/AudioSourceUserData.h"
#include "ScriptNamespace/Classes/Audio/AudioBufferUserData.h"

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
#include "ScriptNamespace/Classes/Ogre/Scene/CameraUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/Skeleton/SkeletonUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/Skeleton/SkeletonAnimationUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/Skeleton/BoneUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/AabbUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/RayUserData.h"
#include "ScriptNamespace/Classes/Ogre/Scene/TerrainObjectUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/StagingTextureUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/TextureBoxUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/GPUProgramUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/GPUProgramParametersUserData.h"
#include "ScriptNamespace/Classes/Ogre/Hlms/PassBufferUserData.h"
#include "ScriptNamespace/Classes/Ogre/Hlms/PassPropertiesUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/MaterialUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/MeshUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/SubMeshUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/VertexArrayObjectUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/OgreBufferUserData.h"
#include "ScriptNamespace/Classes/Ogre/Graphics/VertexElementVecUserData.h"
#include "ScriptNamespace/Classes/Ogre/Compositor/CompositorWorkspaceUserData.h"
#include "ScriptNamespace/Classes/Animation/AnimationInstanceUserData.h"

#include "ScriptNamespace/Classes/Vector3UserData.h"
#include "ScriptNamespace/Classes/Vector2UserData.h"
#include "ScriptNamespace/Classes/ColourValueUserData.h"
#include "ScriptNamespace/Classes/QuaternionUserData.h"
#include "ScriptNamespace/Classes/PlaneUserData.h"
#include "ScriptNamespace/Classes/CollisionWorldClass.h"
#include "ScriptNamespace/Classes/Lottie/LottieAnimationUserData.h"
#include "ScriptNamespace/Classes/Lottie/LottieSurfaceUserData.h"

#include "ScriptNamespace/Classes/Util/XMLDocumentUserData.h"
#include "ScriptNamespace/Classes/Util/XMLElementUserData.h"

#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"
#include "System/SystemSetup/SystemSettings.h"

#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>
#include <sqstdstring.h>
#include <sqstdblob.h>

#include "ScriptManager.h"
#include "System/BaseSingleton.h"
#include "Window/Window.h"

#include "Script/Script.h"
#include "Script/CallbackScript.h"

#ifdef DEBUGGING_TOOLS
    #include "Debugger/ScriptDebugger.h"
#endif
#if defined(DEBUGGING_TOOLS) || defined(SCRIPT_PROFILER)
    #include "SquirrelHookDispatcher.h"
#endif
#ifdef SCRIPT_PROFILER
    #include "Profiler/ScriptProfiler.h"
#endif

#include "Logger/Log.h"
#include <cstdio>
#include <stdarg.h>

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

    SQInteger ScriptVM::errorHandler(HSQUIRRELVM vm){
        #ifdef TEST_MODE
            if(ScriptVM::hasTestFinished()) return 0;
        #endif

        std::string errorMessage;
        SQStackInfos si;
        sqLogRuntimeError(vm, "main", &errorMessage, &si);

        #ifdef TEST_MODE
        if(SystemSettings::isTestModeEnabled()){
            //If any scripts fail during a test mode run, the engine is shut down and the test is failed.
            TestingEventScriptFailure event;
            event.srcFile = si.source;
            event.failureReason = errorMessage;
            //event.functionName = si.funcname;
            //event.lineNum = si.line;

            ScriptUtils::_debugBacktrace(_sqvm);

            EventDispatcher::transmitEvent(EventType::Testing, event);
            return 0;
        }
        #endif

        #ifdef DEBUGGING_TOOLS
            if(!SystemSettings::getNoDebugger()){
                mDebugger->pendDebugging();
            }else{
                //Shutdown the engine gracefully when noDebugger flag is set
                BaseSingleton::getWindow()->wantsToClose = true;
            }
        #endif

        return 0;
    }

    static void compilerError(HSQUIRRELVM vm, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column){
        #ifdef TEST_MODE
            if(ScriptVM::hasTestFinished()) return;
        #endif

        sqLogCompilerError("main", desc, source, line, column);

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

        sq_setprintfunc(_sqvm, sqPrintFunc, NULL);

        sq_enabledebuginfo(_sqvm, true);

        #if defined(DEBUGGING_TOOLS) || defined(SCRIPT_PROFILER)
            //Must come before anything which registers a hook consumer, i.e the debugger.
            SquirrelHookDispatcher::initialise(_sqvm);
        #endif

        sq_newclosure(_sqvm, errorHandler, 0);
        sq_seterrorhandler(_sqvm);

        sq_setcompilererrorhandler(_sqvm, compilerError);
    }

    void ScriptVM::initialise(bool useSetupFunction){
        #ifdef TEST_MODE
            EventDispatcher::subscribeStatic(EventType::Testing, AV_BIND_STATIC(ScriptVM::testEventReceiver));
        #endif

        if(useSetupFunction){
            //Attempt to call the setup function in the setup file.
            //Initialise a new VM for the setup function root and environment, which eventually gets wiped and replaced with the main one.
            _initialiseVM();
#ifdef DEBUGGING_TOOLS
            mDebugger = new ScriptDebugger(_sqvm);
#endif
            AV_INFO("Attempting to call SquirrelEntry setup function.");
            _setupSetupFunctionVM(_sqvm);

            //Both must be torn down while the vm they reference is still open.
#ifdef DEBUGGING_TOOLS
            delete mDebugger;
            mDebugger = 0;
#endif
#if defined(DEBUGGING_TOOLS) || defined(SCRIPT_PROFILER)
            SquirrelHookDispatcher::shutdown();
#endif
            sq_close(_sqvm);
        }
        _initialiseVM();
        _setupVM(_sqvm);

        #ifdef DEBUGGING_TOOLS
            mDebugger = new ScriptDebugger(_sqvm);
        #endif

        #ifdef SCRIPT_PROFILER
            //Only the main vm is profiled; the setup function vm is thrown away.
            if(SystemSettings::isScriptProfilerEnabled()){
                ScriptProfilerSettings settings;
                settings.collectLines = SystemSettings::getScriptProfilerLines();
                settings.outputPath = SystemSettings::getScriptProfilerOutputPath();
                ScriptProfiler::initialise(_sqvm, settings);
            }
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

        #ifdef SCRIPT_PROFILER
            //Writes the report and releases the strings it pinned, so it has to run
            //while the vm is still open.
            ScriptProfiler::shutdown();
        #endif
        #ifdef DEBUGGING_TOOLS
            delete mDebugger;
            mDebugger = 0;
        #endif
        #if defined(DEBUGGING_TOOLS) || defined(SCRIPT_PROFILER)
            SquirrelHookDispatcher::shutdown();
        #endif

        sq_close(_sqvm);
        closed = true;
        AV_INFO("Shutdown Squirrel vm.");
    }

    bool ScriptVM::callClosure(HSQOBJECT closure, const HSQOBJECT* context, PopulateFunction func, ReturnFunction retFunc){
        return callClosure(_sqvm, closure, context, func, retFunc);
    }

    bool ScriptVM::callClosure(HSQUIRRELVM vm, HSQOBJECT closure, const HSQOBJECT* context, PopulateFunction func, ReturnFunction retFunc){
        sq_pushobject(vm, closure);
        if(context){
            sq_pushobject(vm, *context);
        }else{
            sq_pushroottable(vm);
        }

        SQInteger paramCount = 1;
        if(func){
            paramCount = (func)(vm);
        }

        if(SQ_FAILED(sq_call(vm, paramCount, true, true))){
            return false;
        }

        if(retFunc){
            (retFunc)(vm);
        }else{
            sq_poptop(vm);
        }

        sq_pop(vm, 1);

        return true;
    }

    void ScriptVM::initialiseScript(Script* s){
        s->initialise(_sqvm);
    }

    void ScriptVM::initialiseCallbackScript(CallbackScript *s){
        s->initialise(_sqvm);
    }

    SQRESULT ScriptVM::populateStackInfoLowestFrame(SQStackInfos* stackInfo){
        return SQ_SUCCEEDED(sq_stackinfos(_sqvm, 1, stackInfo));
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

    void ScriptVM::setupNamespace(const char* namespaceName, NamespaceSetupFunction setupFunc){
        setupNamespace(_sqvm, namespaceName, setupFunc);
    }

    void ScriptVM::setupNamespace(HSQUIRRELVM vm, const char* namespaceName, NamespaceSetupFunction setupFunc){
        sq_pushstring(vm, _SC(namespaceName), -1);
        sq_newtable(vm);

        setupFunc(vm);

        sq_newslot(vm, -3 , false);
    }

    void ScriptVM::setupDelegateTable(DelegateTableSetupFunction setupFunc){
        (*setupFunc)(_sqvm);
    }

    void ScriptVM::_setupSetupFunctionVM(HSQUIRRELVM vm){
        sq_pushroottable(vm);

        sqstd_register_mathlib(vm);
        sqstd_register_stringlib(vm);
        sqstd_register_bloblib(vm);

        setupNamespace("_window", WindowNamespace::setupSetupFuncNamespace);
        setupNamespace("_settings", SettingsNamespace::setupSetupFuncNamespace);

        Vector2UserData::setupTable(vm);

        WindowNamespace::setupConstants(vm);
        //The render system and platform constants are needed to call _settings.setRenderSystem().
        SettingsNamespace::setupConstants(vm);

        ScriptUtils::declareConstant(vm, "EXECUTION_SETUP_VM", 1);

        sq_pop(vm,1); //Pop the root table.

        CallbackScriptPtr s = BaseSingleton::getScriptManager()->loadScript(SystemSettings::getSquirrelEntryScriptPath());
        if(s){
            int setupFunction = s->getCallbackId("setup");
            s->call(setupFunction);
        }else{
            AV_ERROR("Unable to call setup function in Squirrel entry script.")
        }
    }

    void ScriptVM::_setupVM(HSQUIRRELVM vm){
        _sqvm = vm;
        //Setup the root table.
        sq_pushroottable(vm);

        sqstd_register_mathlib(vm);
        //sqstd_register_systemlib(vm);
        //sqstd_register_iolib(vm);
        sqstd_register_stringlib(vm);
        sqstd_register_bloblib(vm);

        typedef std::pair<const char*, NamespaceSetupFunction> NamespaceEntry;

        const std::vector<NamespaceEntry> namespaces = {
            {"_camera", CameraNamespace::setupNamespace},
            {"_mesh", MeshNamespace::setupNamespace},
            {"_entity", EntityNamespace::setupNamespace},
            {"_component", ComponentNamespace::setupNamespace},
            {"_scriptingState", ScriptingStateNamespace::setupNamespace},
            {"_input", InputNamespace::setupNamespace},
            {"_settings", SettingsNamespace::setupNamespace},
            {"_physics", PhysicsNamespace::setupNamespace},
            {"_window", WindowNamespace::setupNamespace},
            #ifdef TEST_MODE
                {"_test", TestNamespace::setupNamespace},
            #endif
            {"_hlms", HlmsNamespace::setupNamespace},
            {"_registry", GlobalRegistryNamespace::setupNamespace},
            {"_plugin", PluginNamespace::setupNamespace},
            {"_dataStore", DataStoreNamespace::setupNamespace},
            {"_random", RandomNamespace::setupNamespace},
            {"_gui", GuiNamespace::setupNamespace},
            {"_scene", SceneNamespace::setupNamespace},
            #ifdef DEBUGGING_TOOLS
                {"_developer", DeveloperNamespace::setupNamespace},
            #endif
            {"_event", EventNamespace::setupNamespace},
            {"_animation", AnimationNamespace::setupNamespace},
            {"_state", StateNamespace::setupNamespace},
            {"_compositor", CompositorNamespace::setupNamespace},
            {"_graphics", GraphicsNamespace::setupNamespace},
            {"_system", SystemNamespace::setupNamespace},
            {"_resources", ResourcesNamespace::setupNamespace},
            {"_audio", AudioNamespace::setupNamespace},
            {"_lottie", LottieNamespace::setupNamespace},
            #ifdef ENABLE_MONETISATION
                {"_monetisation", MonetisationNamespace::setupNamespace},
            #endif
        };

        for(const NamespaceEntry& e : namespaces){
            setupNamespace(e.first, e.second);
        }

        //Conditional in the same way _test and _developer are: a project which has not asked for
        //script workers gets no _worker at all, which is also how a script checks for the feature.
        if(SystemSettings::getScriptWorkersEnabled()){
            setupNamespace("_worker", ScriptWorkerNamespace::setupNamespace);
        }

        MiscFunctions::setupFunctions(vm);

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
        CameraUserData::setupDelegateTable(vm);
        SkeletonUserData::setupDelegateTable(vm);
        SkeletonAnimationUserData::setupDelegateTable(vm);
        BoneUserData::setupDelegateTable(vm);
        AabbUserData::setupDelegateTable(vm);
        RayUserData::setupDelegateTable(vm);
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
        PassBufferUserData::setupDelegateTable(vm);
        PassPropertiesUserData::setupDelegateTable(vm);
        CompositorWorkspaceUserData::setupDelegateTable(vm);
        EntityUserData::setupDelegateTable(vm);
        AudioSourceUserData::setupDelegateTable(vm);
        AudioBufferUserData::setupDelegateTable(vm);
        PlaneUserData::setupDelegateTable(vm);
        VertexElementVecUserData::setupDelegateTable(vm);
        if(SystemSettings::getScriptWorkersEnabled()){
            ScriptWorkerHandleUserData::setupDelegateTable(vm);
        }
        XMLDocumentUserData::setupDelegateTable(vm);
        XMLElementUserData::setupDelegateTable(vm);
        CollisionWorldClass::setupDelegateTable(vm);
        LottieAnimationUserData::setupDelegateTable(vm);
        LottieSurfaceUserData::setupDelegateTable(vm);
        TerrainObjectUserData::setupDelegateTable(vm);

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
        CollisionWorldClass::setupConstants(vm);
        if(SystemSettings::getScriptWorkersEnabled()){
            ScriptWorkerNamespace::setupConstants(vm);
        }

        ScriptUtils::declareConstant(vm, "EXECUTION_SETUP_VM", 0);
        //So a file shared between this vm and a worker vm can tell which one it is running in.
        ScriptUtils::declareConstant(vm, "EXECUTION_WORKER_VM", 0);

        sq_pop(vm,1); //Pop the root table.
    }
}
