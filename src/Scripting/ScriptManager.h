#pragma once

#include <squirrel.h>
#include <string>

namespace Ogre{
    class Camera;
    class SceneManager;
};

namespace AV {
    class CameraNamespace;
    class MeshNamespace;
    class WorldNamespace;
    class SlotManagerNamespace;
    class TestNamespace;
    class EntityNamespace;
    class Script;

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
         Run a single script.

         @arg scriptPath
         The path of the target script.
         */
        static void runScript(const std::string &scriptPath);

        static void callFunction(const std::string &scriptPath, const std::string &functionName);

        static void initialiseScript(Script* s);

        /**
        Inject the script manager with its required pointers. This should be done early in the engine startup.
        */
        static void injectPointers(Ogre::Camera *camera, Ogre::SceneManager* sceneManager);

    private:
        //Whether or not the script manager has been closed.
        static bool closed;
        /**
         The Squirrel Virtual Machine used for running scripts.
         */
        static HSQUIRRELVM _sqvm;

        /**
         Internally setup the class.
         */
        static void _setupVM(HSQUIRRELVM vm);

        static void _createCameraNamespace(HSQUIRRELVM vm, CameraNamespace &cameraNamespace);

        static void _createMeshNamespace(HSQUIRRELVM vm, MeshNamespace &meshNamespace);

        static void _createWorldNamespace(HSQUIRRELVM vm, WorldNamespace &worldNamespace);

        static void _createSlotManagerNamespace(HSQUIRRELVM vm, SlotManagerNamespace &slotManagerNamespace);

        static void _createTestNamespace(HSQUIRRELVM vm, TestNamespace &testNamespace);

        static void _createEntityNamespace(HSQUIRRELVM vm, EntityNamespace &entityNamespace);

        static void _createVec3Class(HSQUIRRELVM vm);

        static void _createSlotPositionClass(HSQUIRRELVM vm);

        static void _setupEntityClass(HSQUIRRELVM vm);

        static void _createComponentNamespace(HSQUIRRELVM vm);

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
