#pragma once

#include "ScriptNamespace.h"

namespace Ogre{
    class Camera;
};

namespace AV{
    class ScriptManager;
    class CameraNamespace : public ScriptNamespace{
        friend ScriptManager;
    public:
        CameraNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        /**
        The camera which the scripts will use.
        This should be set by the injection in the script manager.
        */
        static Ogre::Camera *_camera;

        static SQInteger setCameraPosition(HSQUIRRELVM vm);
        static SQInteger cameraLookat(HSQUIRRELVM vm);
    };
}
