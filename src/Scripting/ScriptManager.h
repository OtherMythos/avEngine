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

    class ScriptManager{
    public:
        /**
         Call to initialise the Script Manager. This should be done at startup.
         */
        static void initialise();

        /**
         Run a single script.

         @arg scriptPath
         The path of the target script.
         */
        static void runScript(const std::string &scriptPath);

        /**
        Inject the script manager with its required pointers.
        */
        static void injectPointers(Ogre::Camera *camera, Ogre::SceneManager* sceneManager);

    private:
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
    };
}
