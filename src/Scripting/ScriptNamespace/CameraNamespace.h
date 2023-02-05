#pragma once

#include "ScriptUtils.h"

namespace Ogre{
    class Camera;
};

namespace AV{
    class ScriptVM;
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

        static SQInteger setCameraPosition(HSQUIRRELVM vm);
        static SQInteger cameraLookat(HSQUIRRELVM vm);
        static SQInteger setOrientation(HSQUIRRELVM vm);
        static SQInteger setDirection(HSQUIRRELVM vm);
        static SQInteger getCameraPosition(HSQUIRRELVM vm);
        static SQInteger getCameraOrientation(HSQUIRRELVM vm);
        static SQInteger setProjectionType(HSQUIRRELVM vm);
        static SQInteger setOrthoWindow(HSQUIRRELVM vm);
        static SQInteger setAspectRatio(HSQUIRRELVM vm);

        static SQInteger getCameraToViewportRay(HSQUIRRELVM vm);
        static SQInteger getDefaultCamera(HSQUIRRELVM vm);
        static SQInteger getDirection(HSQUIRRELVM vm);
    };
}
