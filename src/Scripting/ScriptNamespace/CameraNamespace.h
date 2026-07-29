#pragma once

#include "ScriptUtils.h"

namespace Ogre{
    class Camera;
};

namespace AV{
    class ScriptVM;
    /**
     * The _camera script namespace.
     *
     * Functions here operate on the engine's default camera. The shared camera
     * functions are implemented once in CameraUserData, taking an Ogre::Camera
     * directly; the wrappers below just pass _camera through to them. Only the
     * genuinely namespace-specific functions are implemented here.
     */
    class CameraNamespace{
        friend ScriptVM;
    public:
        CameraNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

    private:
        /**
        The camera which the scripts will use.
        This should be set by the injection in the script manager.
        */
        static Ogre::Camera *_camera;

        //Namespace-specific functions.
        static SQInteger setCameraPosition(HSQUIRRELVM vm);
        static SQInteger getCameraPosition(HSQUIRRELVM vm);
        static SQInteger setOrientation(HSQUIRRELVM vm);
        static SQInteger getDirection(HSQUIRRELVM vm);
        static SQInteger getDefaultCamera(HSQUIRRELVM vm);

        //Wrappers forwarding onto the default camera object. Implementations live in CameraUserData.
        static SQInteger cameraLookat(HSQUIRRELVM vm);
        static SQInteger setDirection(HSQUIRRELVM vm);
        static SQInteger getCameraOrientation(HSQUIRRELVM vm);
        static SQInteger setProjectionType(HSQUIRRELVM vm);
        static SQInteger setOrthoWindow(HSQUIRRELVM vm);
        static SQInteger setAspectRatio(HSQUIRRELVM vm);
        static SQInteger setFOVy(HSQUIRRELVM vm);
        static SQInteger getCameraToViewportRay(HSQUIRRELVM vm);
    };
}
