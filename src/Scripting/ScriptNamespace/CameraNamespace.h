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

    private:
        /**
        The camera which the scripts will use.
        This should be set by the injection in the script manager.
        */
        static Ogre::Camera *_camera;

        static SQInteger setCameraPosition(HSQUIRRELVM vm);
        static SQInteger cameraLookat(HSQUIRRELVM vm);
        static SQInteger setOrientation(HSQUIRRELVM vm);
    };
}
