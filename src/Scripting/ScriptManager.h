#pragma once

#include <squirrel.h>
#include <string>

namespace Ogre{
    class Camera;
    class SceneManager;
};

namespace AV {
    class Script;
    class CallbackScript;
    class ScriptNamespace;
    class ScriptingStateManager;
    class Event;
    class ScriptDebugger;

    /**
     Manage the creation and execution of squirrel VMs and scripts.
     */
    class ScriptManager{
    public:
        /**
         Call to initialise the Script Manager. This should be done at startup.
         */
        static void initialise();

        /**
        Close and shutdown the script manager and the squirrel vm.
        */
        static void shutdown();

        /**
        Initialise a script with the vm of the Script Manager.
        */
        static void initialiseScript(Script* s);
        /**
        Initialise a callback script with the vm of the Script Manager.
        */
        static void initialiseCallbackScript(CallbackScript *s);

        /**
        Inject the script manager with its required pointers. This should be done early in the engine startup.
        */
        static void injectPointers(Ogre::Camera *camera, Ogre::SceneManager* sceneManager, ScriptingStateManager* stateManager);

        #ifdef TEST_MODE
            static bool testEventReceiver(const Event &e);

            static bool hasTestFinished() { return testFinished; }
        #endif


    private:
        //Whether or not the script manager has been closed.
        static bool closed;

        #ifdef TEST_MODE
            static bool testFinished;
        #endif
        /**
         The Squirrel Virtual Machine used for running scripts.
         */
        static HSQUIRRELVM _sqvm;

        #ifdef DEBUGGING_TOOLS
            static ScriptDebugger* mDebugger;
        #endif

        /**
        Called internally to setup aspects such as the print function, error functions, etc.
        */
        static void _initialiseVM();

        /**
         Internally setup the class.
         */
        static void _setupVM(HSQUIRRELVM vm);

        static void _setupConstants(HSQUIRRELVM vm);

        static void _declareConstant(HSQUIRRELVM vm, const char* name, SQInteger val);

        static void _processSquirrelFailure(const std::string& scriptPath);

        static void _debugStack(HSQUIRRELVM sq);

        static const char* typeToStr(SQObjectType type) {
            switch (type) {
                case OT_INTEGER: return "INTEGER";
                case OT_FLOAT: return "FLOAT";
                case OT_BOOL: return "BOOL";
                case OT_STRING: return "STRING";
                case OT_TABLE: return "TABLE";
                case OT_ARRAY: return "ARRAY";
                case OT_USERDATA: return "USERDATA";
                case OT_CLOSURE: return "CLOSURE";
                case OT_NATIVECLOSURE: return "NATIVECLOSURE";
                case OT_GENERATOR: return "GENERATOR";
                case OT_USERPOINTER: return "USERPOINTER";
                case OT_THREAD: return "THREAD";
                case OT_FUNCPROTO: return "FUNCPROTO";
                case OT_CLASS: return "CLASS";
                case OT_INSTANCE: return "INSTANCE";
                case OT_WEAKREF: return "WEAKREF";
                case OT_OUTER: return "OUTER";
                default: return "UNKNOWN";
            }
        }

    };
}
