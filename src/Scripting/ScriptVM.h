#pragma once

#include <squirrel.h>

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

    //Function which can be called to populate a closure call.
    typedef SQInteger(*PopulateFunction)(HSQUIRRELVM vm);

    /**
     Manage the creation and execution of squirrel VMs.

     This class creates a virtual machine, and contains the procedure to set it up correctly, with the appropriate content in the root table.
     */
    class ScriptVM{
        friend class ScriptEventManager;
    public:
        /**
         Call to initialise the virtual machine. This should be done at startup.
         */
        static void initialise();

        /**
        Close and shutdown the squirrel vm.
        */
        static void shutdown();

        /**
        Initialise a script with the vm.
        */
        static void initialiseScript(Script* s);
        /**
        Initialise a callback script with the vm.
        */
        static void initialiseCallbackScript(CallbackScript *s);

        /**
        Call a squirrel closure, optionally passing a populate function.
        */
        static bool callClosure(HSQOBJECT closure, const HSQOBJECT* context = 0, PopulateFunction populateFunc = 0);

        /**
        Inject functions in the virtual machine with its required pointers. This should be done early in the engine startup.
        */
        static void injectPointers(Ogre::Camera *camera, Ogre::SceneManager* sceneManager, ScriptingStateManager* stateManager);

        /**
        Increase the reference count of a closure by 1.
        */
        static void referenceObject(SQObject closure);
        /**
        Decrease the reference count of a closure by 1.
        */
        static void dereferenceObject(SQObject closure);

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

        static SQInteger errorHandler(HSQUIRRELVM vm);

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

    };
}
