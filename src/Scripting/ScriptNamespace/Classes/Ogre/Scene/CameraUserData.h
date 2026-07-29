#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class Camera;
}

namespace AV{
    /**
     * Exposes Ogre::Camera to scripts.
     *
     * The camera logic is implemented once per function, taking the target Ogre::Camera
     * directly. The delegate table (for camera objects) and CameraNamespace (for the
     * default camera) each provide a thin wrapper that obtains the camera pointer and
     * calls straight into this shared implementation - no argument stack rebuilding
     * needed, since both entry points already place their arguments from stack index 2.
     */
    class CameraUserData{
    public:
        CameraUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void cameraToUserData(HSQUIRRELVM vm, Ogre::Camera* cam);

        //Shared implementations. Callable directly with any Ogre::Camera, so CameraNamespace
        //can forward to these using its own _camera without going through userdata at all.
        static SQInteger cameraLookAt(HSQUIRRELVM vm, Ogre::Camera* cam);
        static SQInteger cameraSetProjectionType(HSQUIRRELVM vm, Ogre::Camera* cam);
        static SQInteger cameraSetOrthoWindow(HSQUIRRELVM vm, Ogre::Camera* cam);
        static SQInteger cameraGetWorldPosInWindow(HSQUIRRELVM vm, Ogre::Camera* cam);
        static SQInteger cameraSetAspectRatio(HSQUIRRELVM vm, Ogre::Camera* cam);
        static SQInteger cameraGetCameraToViewportRay(HSQUIRRELVM vm, Ogre::Camera* cam);
        static SQInteger cameraSetDirection(HSQUIRRELVM vm, Ogre::Camera* cam);
        static SQInteger cameraGetOrientation(HSQUIRRELVM vm, Ogre::Camera* cam);
        static SQInteger cameraSetNearClipDistance(HSQUIRRELVM vm, Ogre::Camera* cam);
        static SQInteger cameraSetFarClipDistance(HSQUIRRELVM vm, Ogre::Camera* cam);
        static SQInteger cameraSetFOVy(HSQUIRRELVM vm, Ogre::Camera* cam);

    private:
        static SQObject cameraDelegateTableObject;

        //Read the camera object from the userdata at stack index 1.
        static Ogre::Camera* _readCamera(HSQUIRRELVM vm);

        //Delegate table adapters: read the camera from self (stack index 1), then call
        //the shared implementation above.
        static SQInteger _lookAt(HSQUIRRELVM vm);
        static SQInteger _setProjectionType(HSQUIRRELVM vm);
        static SQInteger _setOrthoWindow(HSQUIRRELVM vm);
        static SQInteger _getWorldPosInWindow(HSQUIRRELVM vm);
        static SQInteger _setAspectRatio(HSQUIRRELVM vm);
        static SQInteger _getCameraToViewportRay(HSQUIRRELVM vm);
        static SQInteger _setDirection(HSQUIRRELVM vm);
        static SQInteger _getOrientation(HSQUIRRELVM vm);
        static SQInteger _setNearClipDistance(HSQUIRRELVM vm);
        static SQInteger _setFarClipDistance(HSQUIRRELVM vm);
        static SQInteger _setFOVy(HSQUIRRELVM vm);
    };
}
